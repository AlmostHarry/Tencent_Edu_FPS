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
