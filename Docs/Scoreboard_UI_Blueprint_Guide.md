# Tab Scoreboard Blueprint Guide

The C++ side now provides a replicated scoreboard data path and a Widget Blueprint parent.

## Required Widget Blueprint

Create:

- Path: `/Game/Variant_Shooter/UI/WBP_Scoreboard`
- Parent Class: `EduScoreboardWidget`

Then configure the player controller:

- Open the Blueprint class used by the shooter player controller, for example `BP_ShooterPlayerController`.
- Open `Class Defaults`.
- Find `Shooter | UI > Scoreboard Widget Class`.
- Set it to `WBP_Scoreboard`.
- Compile and save.

Suggested simple Widget Tree:

- Root: `Canvas Panel`
- Child: `Border` or `Overlay` centered near the top/middle of the screen
- Child inside it: `TextBlock`
  - Name: `ScoreboardText`
  - Check `Is Variable`
  - Enable wrapping or use a monospace-looking font if desired
  - Initial text can be:

```text
RED TEAM
Slot  Name        K / D / A
R1    R1 Player   0 / 0 / 0
R2    R2 AI       0 / 0 / 0

BLUE TEAM
Slot  Name        K / D / A
B1    B1 Player   0 / 0 / 0
B2    B2 AI       0 / 0 / 0
```

If the TextBlock is named exactly `ScoreboardText`, C++ updates it automatically.

## Optional Graph Setup

Use this only if you want a custom graph-driven presentation instead of the automatic `ScoreboardText` update.

1. Open the Graph tab.
2. Right-click and search for `Scoreboard Updated`.
3. Add `Event Scoreboard Updated`.
4. Drag your TextBlock variable into the graph as `Get`.
5. Drag from the TextBlock pin and add `SetText (Text)`.
6. Connect:
   - `Event Scoreboard Updated` exec pin to `SetText`
   - `Display Text` pin to `SetText` `In Text`
   - TextBlock variable to `SetText` `Target`
7. Compile and save.

No input asset is needed. `ShooterPlayerController` binds keyboard `Tab` directly:

- Press Tab: create and show `WBP_Scoreboard`
- Release Tab: remove it from the screen

## C++ Data Source

`EduShooterGameState` replicates an array of `FEduScoreboardEntry`:

- `Selection`: team and slot, such as `R1` or `B2`
- `DisplayName`: current occupant label, such as `R1 Player` or `B2 AI`
- `bHuman`: whether the slot is occupied by a human player
- `Kills`
- `Deaths`
- `Assists`

The Blueprint event receives both:

- `Entries`: raw replicated entries for a future custom table UI
- `Display Text`: a ready-to-display text table for the simple MVP version
