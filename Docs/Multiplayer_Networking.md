# Multiplayer Networking Notes

## Purpose

This document records the first server-authoritative networking pass for the Shooter MVP, the
problems found while converting the original single-player template, and the rules that should be
preserved during later multiplayer work.

The project uses Unreal Engine replication with a listen server. It is state replication with
server authority, not deterministic frame synchronization.

## Current Architecture

### Match setup

- The listen-server host is the only player allowed to select single-player or two-player mode.
- The selected mode and match-start state live in `EduShooterGameState` and replicate to clients.
- Team-slot requests are sent to the server and validated by `ShooterGameMode`.
- Slot ownership is stored per player in `EduShooterPlayerState`.
- In two-player mode, the match waits until two human players have selected valid slots.
- Before the match starts, waiting pawns are hidden, have collision disabled, and cannot move or fire.
- Empty team slots are filled by server-owned AI. A joining human can replace the AI in the requested
  slot.

### Authority ownership

| System | Authority |
| --- | --- |
| Match mode and match start | Server `GameMode`, replicated through `GameState` |
| Team-slot validation | Server `GameMode` |
| Per-player selected slot | Server, replicated through `PlayerState` |
| AI spawning and decisions | Server |
| Player input and local UI | Owning client |
| Shot request timing | Owning client |
| Shot-rate validation | Server weapon |
| Projectile spawn and movement source | Server, replicated to clients |
| Ammo, damage, death, score, victory | Server |
| Camera, first-person presentation, and local HUD | Owning client only |

### Human shooting flow

1. `DoStartFiring` immediately requests one shot.
2. If the weapon is full-auto, the locally controlled character schedules further requests using the
   weapon's existing `RefireRate`.
3. `DoStopFiring` immediately clears the local request timer.
4. Each request sends the current camera trace target to `ServerFireShot`.
5. The server clamps and stores the aim target, verifies gameplay state, and calls
   `ShooterWeapon::TryFireOnce`.
6. `TryFireOnce` rejects requests that arrive before `RefireRate` has elapsed.
7. Only the server consumes ammo and spawns the replicated projectile.

This is intentionally different from keeping a full-auto timer running on the server between a
`StartFiring` RPC and a later `StopFiring` RPC.

AI does not need client input RPCs. AI shooting remains fully server-side and can continue using the
weapon's server timer.

## Problems Encountered

### Match started after only one player selected a slot

**Symptom:** Either player selecting a slot immediately entered gameplay, while the other player was
still selecting or the world appeared empty.

**Cause:** Match start was not gated by the number of human players required by the selected mode.

**Resolution:** The server now derives the required player count from `EduShooterGameState` and starts
only after enough valid human slot selections exist. Waiting clients receive a dedicated
"waiting for the other player" UI state.

### Extra red, blue, or white characters

**Symptom:** The level contained the managed 2v2 population plus an unexpected default or waiting
player pawn.

**Cause:** Template level NPC spawners, managed slot AI, and newly connected player pawns could all
exist at the same time. A player pawn was also visible before it owned a valid match slot.

**Resolution:** Managed matches disable the old level spawners. Waiting player pawns are hidden and
non-colliding. A human claiming a slot replaces that slot's AI.

### Controller-null Blueprint runtime errors after weapon pickup

**Symptom:** Picking up a weapon on one network instance produced repeated Blueprint errors involving
`GetController` and pitch updates.

**Cause:** Replicated remote pawns exist on every machine, but only the owning pawn has the local
controller and camera state expected by first-person template logic.

**Resolution and rule:** First-person animation, camera, controller pitch, recoil presentation, and
local HUD work must run only for the locally controlled pawn and must validate controller pointers.
Pickup overlap and weapon granting are server-only.

### Full-auto weapon sometimes produced an unwanted second shot

**Symptom:** A short click could produce two rifle projectiles, especially after networking was added.
The issue could also be observed in listen-server single-player because that mode still follows the
network-authoritative code path.

**Original template behavior:** The rifle asset has always been full-auto with a `RefireRate` of
approximately `0.1` seconds. The root commit `be0b36f` already fired a second shot if the input
remained held beyond that interval. The rifle asset, `IA_Shoot`, and `IMC_Weapons` Git blobs have not
changed since the root commit.

**Networking regression:** The first network implementation sent `StartFiring` and `StopFiring` RPCs
and ran the full-auto timer on the server. The stop RPC could arrive after the next server timer
deadline, adding an extra shot beyond the local input intent.

**Current resolution:** Human players now send individual shot requests. Releasing input immediately
stops local requests, while the server independently enforces `RefireRate`. Weapon fire rate was not
changed to hide the networking issue.

**Expected behavior:** A rifle press held for at least one `RefireRate` interval can still produce a
second shot because this is the original full-auto behavior. If design later requires forgiving
single-shot taps, add a separate local hold threshold before starting automatic follow-up requests.
Do not implement that threshold by changing `RefireRate`.

### Shots deviated far from the crosshair

**Symptom:** Both host and client projectiles could diverge from the crosshair far more than expected.

**Cause:** During the networking conversion, existing `AimVariance` asset values such as `15`, `20`,
and `30` were incorrectly passed to an angular cone function as degrees. In the original template,
those values offset the target position in world-space distance units.

**Resolution:** Preserve target-space variance and aim the authoritative projectile from the selected
muzzle toward that varied target. Do not reinterpret the existing values as degrees and do not edit
the weapon assets to compensate.

### Wrong muzzle or first-person logic on remote players

**Symptom:** Remote shots or weapon presentation could use first-person-only state that is unavailable
or inappropriate on the server.

**Cause:** The original template assumed one local player and always used first-person components.

**Resolution:** The locally controlled listen-server pawn may use its first-person muzzle. Remote
players use the replicated third-person weapon mesh on the authoritative server. First-person meshes
and animation remain owner-only presentation.

### PIE warning about login credentials

**Symptom:** PIE reported that there were not enough login credentials to launch every instance.

**Cause:** This is an editor multiplayer-instance configuration warning, not a gameplay replication
failure.

**Action:** For local listen-server testing, use one listen server and one client without requiring
online subsystem credentials. Treat actual Blueprint runtime errors, failed joins, or missing
replicated state separately from this editor warning.

### Existing map missing-asset warnings

**Symptom:** The Shooter map reported two missing asset references while multiplayer work was being
tested.

**Cause:** These were content/reference problems discovered during integration, not replication
failures. A file being unchanged in Git does not prove that every package it references exists in the
repository.

**Resolution and rule:** The required material/map references were repaired and the affected packages
were resaved. Keep load-time asset warnings separate from networking diagnosis, and continue leaving
the unused imported `Content/XL_FPSPack/` directory untracked.

## Template Versus Networking Responsibility

The UE Shooter variant was designed primarily as a local template. Its weapon values and basic
full-auto behavior are valid starting points, but assumptions about one local controller, one
first-person camera, and direct local actor mutation do not automatically work in multiplayer.

When a regression appears after networking:

1. Compare against root commit `be0b36f`.
2. Check whether the asset blob actually changed.
3. Separate original weapon design from RPC timing, ownership, or authority errors.
4. Fix the network path before tuning weapon values.
5. Verify listen-server local play and remote-client play because they exercise different ownership
   branches.

## PIE Test Procedure

Recommended settings:

- Players: `2`
- Net mode: `Play As Listen Server`
- Run under one process when practical for fast iteration
- Use separate windows when identifying input ownership

Do not identify server and client by window size alone. Check the PIE title and logs:

- A window titled `NetMode: Client 1` is the client.
- The editor/listen-server world is the host.

Regression pass:

1. Confirm only the host can choose match mode.
2. Select two-player mode and verify both players remain in setup until both choose different slots.
3. Confirm the managed population contains exactly four active match slots.
4. Pick up a weapon from both host and client and check the Output Log for Blueprint errors.
5. Tap the pistol and verify one authoritative projectile per press.
6. Tap and hold the rifle from both host and client. Verify local release stops new requests and that
   the server respects `RefireRate`.
7. Compare projectile paths with the crosshair at short and long range.
8. Verify damage, death, team score, respawn, victory, and restart on both instances.
9. Repeat shooting tests with PIE packet lag and packet loss simulation.

## Remaining Hardening

The MVP server clamps client-provided aim targets to maximum aim distance and validates fire rate.
Future production hardening could also validate view-angle changes, line of sight, shot sequence
numbers, packet loss behavior, and client-side cosmetic prediction. These are not required for the
current course MVP.
