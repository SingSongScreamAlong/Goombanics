# GOOMBANICS: COLLATERAL DAMAGE

A local co-op (1-4 players) third-person shooter where players battle a giant Kaiju while the city crumbles around them.

## Engine Requirements

- **Unreal Engine 5.5.x** (same version on all development machines)
- macOS and Windows cross-platform compatible

## Project Structure

```
Goombanics/
├── Config/                          # Engine and game configuration
├── Content/
│   ├── Blueprints/
│   │   ├── Characters/              # Player character blueprints
│   │   ├── Monsters/                # Kaiju blueprints
│   │   ├── Weapons/                 # Weapon blueprints
│   │   └── UI/                      # HUD and menu widgets
│   ├── Input/                       # Enhanced Input assets
│   └── Maps/                        # Level maps
└── Source/Goombanics/
    ├── Core/                        # GameMode, GameState, Types
    ├── Player/                      # Character, PlayerState
    ├── Monster/                     # MonsterBase, KaijuPawn
    ├── Weapons/                     # WeaponComponent, Projectile
    ├── Destruction/                 # BreakableActor
    └── UI/                          # HUD, Widgets
```

## Core Classes

| Class | Purpose |
|-------|---------|
| `AGoombanicsGameMode` | Match flow, player spawning, local player creation |
| `AGoombanicsGameState` | Replicated match state (timer, destruction, Kaiju health) |
| `AGoombanicsPlayerState` | Per-player scoring and role assignment |
| `AGoombanicsCharacter` | Player pawn with movement, dash, weapon component |
| `AGoombanicsMonsterBase` | Abstract monster base with weak point system |
| `AGoombanicsKaijuPawn` | Kaiju implementation with AI, attacks, stagger |
| `UGoombanicsWeaponComponent` | Modular weapon system (hitscan + projectile) |
| `AGoombanicsBreakableActor` | Destructible objects contributing to destruction meter |
| `UGoombanicsHUDWidget` | In-match HUD with timer, meters, scoreboard |

## Key Features

### Local Split-Screen (1-4 Players)
```cpp
// In Blueprint or C++
AGoombanicsGameMode* GM = GetWorld()->GetAuthGameMode<AGoombanicsGameMode>();
GM->CreateLocalPlayers(4); // Creates 4 local players
```

### Kaiju Weak Point System
- **LeftLeg** and **RightLeg**: Destroy both to trigger stagger
- **Head**: 2x damage multiplier during stagger state
- Weak points tracked via component tags: `WeakPoint_LeftLeg`, `WeakPoint_RightLeg`, `WeakPoint_Head`

### Match End Conditions
1. **Kaiju Defeated**: Kaiju health reaches 0
2. **City Destroyed**: Destruction meter reaches 100%
3. **Timer Expired**: Default 180 seconds

### Scoring System (Data-Driven)
```cpp
FGoombanicsScoreWeights Weights;
Weights.KaijuDamageWeight = 1.0f;
Weights.CollateralDamageWeight = 0.5f;
Weights.WeakPointDestroyedBonus = 500.0f;
Weights.FinalBlowBonus = 1000.0f;
Weights.DeathPenalty = 100.0f;
```

## Running the Project

### First-Time Setup

1. **Open in Unreal Engine 5.5.x**
   - File → Open Project → Select `Goombanics.uproject`
   - Wait for shader compilation

2. **Generate Project Files** (if needed)
   - Right-click `Goombanics.uproject` → Generate Project Files
   - Open in Visual Studio (Windows) or Xcode (macOS)

3. **Build the Project**
   - Build → Build Solution (Development Editor)

### PIE Testing (1-4 Local Players)

1. **Single Player**
   - Play → Selected Viewport (or Alt+P)

2. **Multiple Local Players**
   - Edit → Editor Preferences → Level Editor → Play
   - Set "Number of Players" to desired count (1-4)
   - Enable "Run Under One Process"
   - Play → New Editor Window (PIE)

3. **Via Blueprint/Console**
   ```
   // Console command
   Goombanics.CreateLocalPlayers 4
   ```

### Listen Server Testing

1. **Host**
   - Play → Play As Listen Server

2. **Join**
   - Open another editor instance
   - Play → Play As Client
   - Connect to: `127.0.0.1`

### Standalone Testing

```bash
# macOS
./Goombanics.app/Contents/MacOS/Goombanics -game -log

# Windows
Goombanics.exe -game -log
```

## Blueprint Setup Required

After opening the project, create these Blueprint assets:

### 1. Input Mapping Context
`Content/Input/IMC_Default.uasset`
- Move: WASD / Left Stick
- Look: Mouse / Right Stick
- Jump: Space / A Button
- Fire: Left Mouse / Right Trigger
- Dash: Shift / B Button
- Switch Weapon: Q / Y Button

### 2. Player Character Blueprint
`Content/Blueprints/Characters/BP_GoombanicsCharacter.uasset`
- Parent: `AGoombanicsCharacter`
- Assign Input Actions and Mapping Context
- Add camera boom and camera component

### 3. Kaiju Blueprint
`Content/Blueprints/Monsters/BP_Kaiju.uasset`
- Parent: `AGoombanicsKaijuPawn`
- Add skeletal mesh
- Configure weak point hitbox positions

### 4. HUD Widget Blueprint
`Content/Blueprints/UI/WBP_HUD.uasset`
- Parent: `UGoombanicsHUDWidget`
- Design layout with bound widgets

### 5. GameMode Blueprint
`Content/Blueprints/Core/BP_GoombanicsGameMode.uasset`
- Parent: `AGoombanicsGameMode`
- Set Default Pawn Class, HUD Class, Kaiju Class

### 6. Map
`Content/Maps/CityBlock_A.umap`
- Add PlayerStart actors
- Add actor with tag "KaijuSpawn"
- Place BreakableActor instances

## Future Architecture Notes

### Player-Controlled Kaiju (PvPvE)
The architecture supports a future mode where one player controls the Kaiju:

```cpp
// Assign Kaiju role to a player
PlayerState->SetPlayerRole(EGoombanicsPlayerRole::Kaiju);

// Possess the Kaiju pawn
KaijuPawn->SetAIEnabled(false);
PlayerController->Possess(KaijuPawn);
```

The `AGoombanicsMonsterBase` class:
- Implements `IGoombanicsMonsterInterface` for consistent API
- Supports both `AIController` and `PlayerController` possession
- All logic lives on the Pawn, not the Controller

### Chaos Destruction (Future)
Current implementation uses logical state-based destruction (`AGoombanicsBreakableActor`).
For Chaos-based setpieces:
1. Create Geometry Collection assets
2. Replace `BrokenMesh` swap with Chaos fracture
3. Hook `OnBroken` to trigger Chaos field

### Online Scaling (8-12 Players)
- `GameState` replication already supports N players
- Scoreboard uses dynamic player list (no hardcoded indices)
- `MaxPlayers=12` configured in `DefaultGame.ini`

## Troubleshooting

### PIE Crashes with Multiple Players
- Ensure "Run Under One Process" is enabled
- Check for null pointer access in BeginPlay (GameState may not exist yet)

### Input Not Working for Player 2+
- Verify `DefaultMappingContext` is assigned in Character Blueprint
- Check `UEnhancedInputLocalPlayerSubsystem` is receiving the context

### Kaiju Not Spawning
- Verify `KaijuPawnClass` is set in GameMode Blueprint
- Check for actor with tag "KaijuSpawn" in level (or uses default location)

### Destruction Meter Not Updating
- Verify `bContributesToDestructionMeter = true` on BreakableActors
- Check `TotalDestructionValue` in GameState (default 10000)

## Version History

- **0.1.0** - Initial vertical slice framework
