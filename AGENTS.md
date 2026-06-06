# AGENTS.md

## Project Context

This is a UE5 first-person shooter course project for Tencent Game "Kai Ju Yi Ke", client direction.

The current goal is to build a small but complete MVP first, then polish only after the demo can already be submitted.

## MVP Scope

The first playable version must include:

1. Player movement, aiming, and shooting based on the UE5 First Person template.
2. Enemies that move toward players.
3. Enemies that attack nearby players.
4. Players can damage and kill enemies.
5. Kills award score.
6. A simple victory condition, such as first player to 10 points.
7. Multiplayer testing through UE PIE with at least 2 players.

Do not add advanced features before this loop works end to end.

## Current Implementation Status

The current playable direction is a 2v2 team match with four logical slots:

- Red team: `R1`, `R2`
- Blue team: `B1`, `B2`
- A local debug UI lets the first local player choose one slot.
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

This implementation is currently single-player/local only. Slot claims, AI population, team state, score,
and material state have not yet been converted into replicated server-authoritative multiplayer systems.

Important implementation files:

- `Source/Tencent_Edu_FPS/Variant_Shooter/EduTeamSlotTypes.h`
- `Source/Tencent_Edu_FPS/Variant_Shooter/UI/EduTeamSelectionWidget.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/ShooterGameMode.*`
- `Source/Tencent_Edu_FPS/Variant_Shooter/ShooterPlayerController.*`
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

If any required `.uasset` or `.umap` file is larger than GitHub's normal file limit, use Git LFS or replace/compress the asset before submission.

## Development Priorities

1. Build the MVP loop.
2. Verify it in single-player.
3. Verify it in PIE multiplayer with 2 players.
4. Fix replication issues.
5. Add UI clarity and simple feedback.
6. Record demo video.
7. Write the PDF with implementation notes and GitHub link.

Avoid large refactors, marketplace asset churn, complex menus, matchmaking, advanced weapon systems, and dedicated server deployment unless the MVP is already stable.

## Verification Checklist

Before considering the project ready:

- The editor opens the `.uproject` without missing required content.
- A default map can be played.
- Two PIE players can join the same session.
- Enemies are visible to all players.
- Enemy damage and death are synchronized.
- Score is synchronized.
- Victory state is synchronized.
- The repo does not include generated UE folders or packaged build output.
