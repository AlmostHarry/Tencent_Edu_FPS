# Day 1 MVP Checklist

Goal: prove the smallest playable shooter loop before adding new features.

Use the existing `Variant_Shooter` template content first. It already includes a player character, weapons, projectiles, NPCs, NPC spawners, health, death, and basic score UI.

## Editor Setup

1. Open `Tencent_Edu_FPS.uproject`.
2. Let Unreal rebuild project files or modules if prompted.
3. Open `/Game/Variant_Shooter/Lvl_Shooter` if it is not opened automatically.
4. Press Play in single-player PIE first.

## What To Verify

The first-day demo is acceptable when all of these work:

1. The player can move and look around.
2. The player can pick up or already hold a weapon.
3. The player can shoot.
4. A projectile or bullet visibly fires.
5. An NPC can take damage.
6. An NPC dies after enough hits.
7. The score UI changes after a kill.

Do not spend time on menus, multiplayer, art polish, new maps, or new weapons today.

## If Something Does Not Work

### The map opens as FirstPerson instead of Shooter

Open `/Game/Variant_Shooter/Lvl_Shooter` manually from the Content Browser.

### Play starts but there is no weapon

In the level, look for `BP_ShooterPickup` actors and walk over one. The Shooter variant uses pickups for weapons.

### The enemy does not move or shoot

Check the level has NavMesh coverage:

1. Press `P` in the editor viewport.
2. Green areas should appear on walkable ground.
3. If there is no green area, add or resize a `NavMeshBoundsVolume` around the arena.

### The enemy cannot be killed

Open `BP_ShooterNPC` and confirm:

1. Its parent class is `ShooterNPC`.
2. `CurrentHP` is not set extremely high.
3. Its collision allows projectile hits.

### The projectile does not damage enemies

Open the weapon projectile Blueprint, usually under:

`/Game/Variant_Shooter/Blueprints/Pickups/Projectiles`

Confirm the projectile parent class is `ShooterProjectile` and `HitDamage` is greater than `0`.

## End Of Day Commit

When the loop works, commit only project/source/config/content changes:

```powershell
git status --short
git add .
git commit -m "Verify day one shooter MVP loop"
```

Ignored folders such as `.vs/`, `Binaries/`, `Intermediate/`, and `Saved/` should not appear as normal untracked files.

## Notes For Tomorrow

Day 2 should focus on making the enemy behavior fit our MVP:

1. Enemy actively chases the closest player.
2. Enemy attacks nearby players.
3. Enemy spawn flow is simple and repeatable.
4. Keep the implementation server-authoritative when multiplayer begins.
