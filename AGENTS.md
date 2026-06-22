# AGENTS.md

## Project Context

This began as a UE5 first-person shooter course project for Tencent Game "Kai Ju Yi Ke", client
direction. The course assignment is complete, and the repository is now in post-course development.

The original MVP is stable and has passed the required single-player and two-player Listen Server PIE
acceptance checks. It should remain a regression baseline, but it is no longer a scope ceiling. New
gameplay systems, UI, networking work, refactors, and polish are allowed when requested; preserve the
working server-authoritative loop and verify affected behavior after each change.

## Original MVP Scope (Completed)

The first playable version must include:

1. Player movement, aiming, and shooting based on the UE5 First Person template.
2. Enemies that move toward players.
3. Enemies that attack nearby players.
4. Players can damage and kill enemies.
5. Kills award score.
6. A simple victory condition, such as first player to 5 points.
7. Multiplayer testing through UE PIE with at least 2 players.

This list is the completed historical baseline, not a restriction on future development.

## Current Implementation Status

The current playable direction is a 2v2 team match with four logical slots:

- Red team: `R1`, `R2`
- Blue team: `B1`, `B2`
- Each local human player uses the setup UI to choose one available slot.
- The chosen slot determines the player's team and tagged `PlayerStart`.
- Unoccupied slots are filled with the existing `BP_ShooterNPC` AI.
- Managed AI slots respawn after death.
- Players respawn at their previously selected slot.
- Existing level NPC spawners are disabled while the slot system manages the 2v2 population.

The level must contain at least two `PlayerStart` actors tagged `RED` and two tagged `BLUE`.
If more exist, the slot system sorts them by world position and uses the first two for each team.

Team identity is stored with `EEduTeam` on the shared character base:

- AI perception only accepts characters on the opposing team as targets.
- Friendly fire between characters on the same assigned team is blocked.
- Kill score is awarded to the killer's team.
- Character materials receive a red or blue runtime tint.
- Default team colors can be changed in `BP_ShooterCharacter` and `BP_ShooterNPC` Class Defaults under `Team > Visuals`.

The server-authoritative multiplayer architecture is implemented:

- The project uses UE state replication with a listen server, not deterministic frame synchronization.
- Human slot selection is requested through a server RPC and validated by `ShooterGameMode`.
- Human players can replace AI occupants in managed match slots.
- Only the listen-server host confirms single-player or two-player mode.
- In PIE, the native `Players` setting determines the only mode button shown to the host:
  - `Players = 1` exposes only single-player mode.
  - `Players = 2` exposes only two-player mode.
- The expected human-player count is replicated through `EduShooterGameState`, and
  `ShooterGameMode` rejects a mode request that does not match it.
- A two-player match does not start until both human players have selected valid slots.
- Pawns waiting for match setup are hidden, non-colliding, and unable to process gameplay input.
- Team and character state replicate from the server.
- Human players request individual shots from their locally controlled pawn.
- The server validates each shot against the weapon `RefireRate` and is the only machine that
  spawns projectiles, consumes ammo, applies damage, and processes death.
- Full-auto timing for human players is driven locally, but every shot still requires server approval.
- AI weapons continue to use server-side hold-to-fire timers.
- AI spawning and decisions remain server-only while NPC movement and death state replicate.
- Team scores and the winning team replicate through `EduShooterGameState`.
- Human team-slot identity replicates through `EduShooterPlayerState`.
- Each local player owns their HUD, team selector, and match result UI.
- A dead human player sees a local respawn countdown driven by the replicated server respawn time.
- Match restart is requested from a client and performed by server travel.
- Kills, deaths, and assists are tracked per human player in `EduShooterPlayerState`.
- Each local player owns a KDA HUD, and the match-result UI receives that player's final KDA.
- Holding `Tab` displays a replicated four-slot scoreboard containing human and AI K/D/A values.
- Jump pads preserve their player launch behavior while supporting per-instance fixed launch direction
  and horizontal/vertical launch speeds for AI.

Important weapon networking constraints:

- Do not restore a player `StartFiring` / `StopFiring` RPC pair that leaves a server-side full-auto
  timer active until the stop RPC arrives. Network delay can produce an unintended extra shot.
- Do not spawn a second predicted gameplay projectile on the owning client. The authoritative
  replicated server projectile is the gameplay projectile.
- Preserve the original Shooter template meaning of `AimVariance`: existing weapon asset values are
  target-space distance offsets, not angular cone values. Do not reinterpret values such as `15`,
  `20`, or `30` as degrees.
- Do not change weapon Blueprint values to hide a replication or input-timing bug. Fix the ownership,
  RPC, or validation path first.
- Controller, camera, first-person mesh, recoil presentation, and local HUD operations must be guarded by local
  ownership or a valid controller. Replicated remote pawns do not own a local controller.

The multiplayer baseline has completed a two-player Listen Server PIE acceptance pass. Shooting from
both players, damage, death, respawn, score, victory, and match restart were verified as working
correctly. Testing under simulated latency or packet loss remains a useful network-hardening task for
future changes.

The current architecture, encountered problems, and regression checklist are documented in
`Docs/Multiplayer_Networking.md`.

Important implementation files:

- `Source/Tencent_Edu_FPS/Variant_Shooter/EduTeamSlotTypes.h`
- `Source/Tencent_Edu_FPS/Variant_Shooter/EduShooterGameState.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/EduShooterPlayerState.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/UI/EduMatchModeWidget.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/UI/EduTeamSelectionWidget.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/UI/EduRespawnCountdownWidget.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/UI/EduKDAWidget.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/UI/EduScoreboardWidget.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/UI/EduMatchResultWidget.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/ShooterGameMode.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/ShooterPlayerController.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/ShooterCharacter.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/Weapons/ShooterWeapon.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/Weapons/ShooterProjectile.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/Weapons/ShooterPickup.*`
- `Source/Tencent_Edu_FPS/Tencent_Edu_FPSCharacter.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/AI/ShooterAIController.*`

## Preferred Implementation

- Prefer Blueprint for fast gameplay iteration.
- Use C++ only when the project already has an established C++ type that clearly benefits from code.
- Keep networking authority on the server:
  - Enemy spawning
  - Enemy AI decisions
  - Damage application
  - Score changes
  - Victory checks
- Let clients handle input, UI display, camera feedback, sound, and visual effects.
- Store per-player score in PlayerState when possible.
- Store match-level replicated state in GameState when possible.
- Keep GameMode server-only.

### C++ Parent and Presentation Blueprint Workflow

For new gameplay or UI types that need both stable logic and editable presentation:

1. Create a C++ parent class for behavior, replicated state, validation, delegates, and Blueprint-facing
   properties or events.
2. Keep layout, styling, animation, asset references, and designer-tunable defaults in a Blueprint child.
3. When editor automation is appropriate, use the Unreal Python Editor API to create and configure the
   presentation Blueprint, including its parent class, components or widget tree, and required names.
4. Treat Python as the asset-creation mechanism, not the runtime architecture. The resulting Blueprint
   should remain editable in the Unreal Editor and should not require Python during gameplay.
5. Compile and save the generated Blueprint, then run Blueprint compilation or project validation before
   considering the asset complete.
6. Commit both the C++ parent and every required `.uasset`; the project must work after a clean checkout.

For Widget Blueprints, use C++ `BindWidget` fields only when the generated or manually created widget tree
can guarantee the required widget names. Expose optional Blueprint events for visual effects and animation
without moving authoritative gameplay logic into the Widget Blueprint.

## Local Unreal Engine

This project currently uses UE 5.7.

Local engine path:

`E:\UnrealEngine\UE_5.7`

Useful editor build command:

```powershell
E:\UnrealEngine\UE_5.7\Engine\Build\BatchFiles\Build.bat Tencent_Edu_FPSEditor Win64 Development -Project="E:\Projects\UEProjects\Tencent_Edu_FPS\Tencent_Edu_FPS.uproject" -WaitMutex
```

If C++ changes add or remove `UPROPERTY`, `UFUNCTION`, `UCLASS`, reflected structs, or class layout used by Blueprints, close the editor and run the full build command instead of relying on Live Coding.

## Git Rules

Commit source assets and project configuration needed to reopen the project:

- `Tencent_Edu_FPS.uproject`
- `Config/`
- `Content/`
- `Source/`
- `.gitignore`
- `AGENTS.md`
- README or documentation files added later

Do not commit generated local files:

- `.vs/`
- `Binaries/`
- `DerivedDataCache/`
- `Intermediate/`
- `Saved/`
- `Backup/`
- `.sln`
- packaged builds
- local recordings

`Content/XL_FPSPack/` contains imported third-party assets that are not currently
used by the playable project. Leave this directory untracked for now; do not add or commit it
unless the project begins depending on those assets.

If any required `.uasset` or `.umap` file is larger than GitHub's normal file limit, use Git LFS or
replace/compress the asset before pushing it.

### Commit and Push Workflow

When asked to commit and push changes:

1. Run `git status --short`, confirm the current branch, and inspect the configured remote.
2. Review the relevant diff before staging. Preserve unrelated user changes and untracked files.
3. Stage only the files that belong to the requested change by listing their paths explicitly. Do not use
   `git add .` when the worktree contains unrelated changes.
4. Check `git diff --cached --check`, `git diff --cached --stat`, and
   `git diff --cached --name-status` before committing.
5. Use a concise imperative commit message that describes the completed change.
6. Push the current branch explicitly, for example `git push origin main`.
7. Verify that local `HEAD` and the remote tracking branch resolve to the same commit after pushing.
8. Report the commit hash, message, push destination, and any changes intentionally left uncommitted.

If `.git/index.lock` blocks a Git write:

- First confirm that no `git` or `git-lfs` process is running.
- Confirm that the lock is the repository's exact `.git/index.lock` path.
- Remove only that stale lock file, then retry the interrupted Git command.
- Never delete other files inside `.git` or reset unrelated worktree changes to solve a lock problem.

## Development Priorities

Completed baseline milestones:

1. Build the MVP loop.
2. Verify it in single-player.
3. Verify it in PIE multiplayer with 2 players.
4. Fix blocking replication issues found during acceptance testing.
5. Add replicated player KDA, local KDA HUD, and final KDA presentation.
6. Add a replicated four-slot Tab scoreboard for humans and AI.
7. Restrict the mode-selection UI and server validation to the configured PIE player count.
8. Add AI-specific jump-pad launch controls and continue gameplay balancing.

Current development policy:

1. Treat the completed MVP as the regression baseline while allowing the project scope to grow.
2. Define acceptance criteria for each requested feature or refactor before considering it complete.
3. Preserve server authority for gameplay state and local ownership for input, camera, HUD, and
   presentation.
4. Update architecture notes and regression checks when behavior or networking assumptions change.
5. Prefer focused, verifiable increments; broad refactors are acceptable when they have a concrete
   benefit and an appropriate verification plan.

Potential future directions include network-emulation hardening, richer startup/session flows, AI and
weapon depth, additional maps or modes, and presentation polish. These are options, not standing tasks;
follow the user's current goal rather than expanding scope automatically.

## Verification Checklist

The baseline acceptance pass is complete. The following checklist should still be used for regression
testing after gameplay or networking changes:

Before considering the project ready:

- The editor opens the `.uproject` without missing required content.
- A default map can be played.
- With PIE `Players = 1`, the host sees only the single-player mode button.
- With PIE `Players = 2`, the host sees only the two-player mode button.
- The server rejects a mode request that does not match the configured PIE player count.
- Two PIE players can join the same session.
- Only the listen-server host can select the match mode.
- A two-player match waits until both players select different valid slots.
- Enemies are visible to all players.
- One trigger press produces no duplicate authoritative projectile.
- Rapid taps and held full-auto fire respect the original weapon `RefireRate`.
- Server and client shots travel toward the crosshair with only the template's configured target-space variance.
- Picking up and switching weapons produces no controller-null Blueprint runtime errors.
- Enemy damage and death are synchronized.
- Score is synchronized.
- Human and AI K/D/A values update correctly on the Tab scoreboard.
- Each human player sees only their own local KDA HUD and final KDA result.
- Victory state is synchronized.
- Each dead human player sees the respawn countdown, and it disappears after possessing the new pawn.
- Respawn and match restart remain synchronized.
- The repo does not include generated UE folders or packaged build output.
