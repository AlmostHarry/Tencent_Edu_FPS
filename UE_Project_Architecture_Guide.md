# Tencent Edu FPS 项目架构与文件职责

本文档说明当前 FPS MVP 中各类文件的大体职责，以及 Unreal Engine 核心对象之间如何协作。

## 1. 常见文件类型

| 类型 | 作用 |
| --- | --- |
| `.h` | 声明 C++ 类、属性、函数、RPC 和 Blueprint 接口 |
| `.cpp` | 实现具体 C++ 逻辑 |
| `.uasset` | Blueprint、UMG、输入、材质、动画、StateTree 等 UE 资产 |
| `.umap` | Unreal Engine 关卡 |
| `.ini` | 项目启动、输入、渲染等配置 |
| `.Build.cs` | 声明 C++ 模块依赖 |
| `.Target.cs` | 定义 Game 或 Editor 构建目标 |

`*.generated.h` 由 Unreal Header Tool 自动生成，为反射、Blueprint、序列化和网络复制提供支持。

## 2. UE 核心对象分工

| 类型 | 项目中的职责 | 网络位置 |
| --- | --- | --- |
| `GameMode` | 比赛规则、出生、AI 填充、计分和胜负 | 仅服务器存在 |
| `GameState` | 全局比分、模式、比赛状态和胜者 | 服务器写入，复制给所有客户端 |
| `PlayerState` | 单个玩家的队伍和槽位身份 | 复制给所有客户端 |
| `PlayerController` | 玩家输入、UI 和服务器 RPC 请求 | 服务器和所属客户端 |
| `Character/Pawn` | 世界中的身体、移动、血量和武器 | 服务器权威并复制 |

最重要的区别是：

- `PlayerController` 代表玩家的控制和本地交互。
- `Character` 代表玩家当前控制的世界角色。
- Character 死亡和替换后，PlayerController 与 PlayerState 可以继续存在。

## 3. 项目入口与构建

### `Tencent_Edu_FPS.uproject`

项目描述文件，声明：

- 使用 Unreal Engine 5.7。
- 加载 `Tencent_Edu_FPS` Runtime 模块。
- 启用 StateTree、GameplayStateTree 等插件。

### `Source/Tencent_Edu_FPS/Tencent_Edu_FPS.Build.cs`

定义 C++ 模块依赖，包括：

- `Engine`
- `EnhancedInput`
- `AIModule`
- `StateTreeModule`
- `GameplayStateTreeModule`
- `UMG`
- `Slate`

代码使用某个 UE 系统前，通常需要在这里添加对应模块。

### `Tencent_Edu_FPS.Target.cs`

定义游戏程序的构建目标。

### `Tencent_Edu_FPSEditor.Target.cs`

定义 Unreal Editor 的构建目标。

### `Tencent_Edu_FPS.h/.cpp`

注册项目主模块，并定义项目使用的日志分类。

## 4. 基础角色层

### `Tencent_Edu_FPSCharacter.h/.cpp`

玩家和 NPC 共用的基础角色类，负责：

- 第一人称相机和手臂。
- 移动、视角和跳跃。
- Enhanced Input 绑定。
- 队伍状态复制。
- 红蓝队伍材质染色。
- 判断另一个角色是否属于敌方。

角色继承关系：

```text
ACharacter
  -> ATencent_Edu_FPSCharacter
       -> AShooterCharacter
            -> BP_ShooterCharacter
       -> AShooterNPC
            -> BP_ShooterNPC
```

C++ 基类定义规则和能力，Blueprint 子类配置模型、动画、输入资产和默认数值。

## 5. 比赛系统

### `Variant_Shooter/EduTeamSlotTypes.h`

定义项目共用的队伍和槽位数据：

- `EEduTeam`：Red、Blue、Unassigned。
- `FEduTeamSlotSelection`：队伍和 1/2 号槽位。

### `Variant_Shooter/ShooterGameMode.h/.cpp`

比赛的服务器裁判，负责：

- 让房主选择单人或双人模式。
- 管理 R1、R2、B1、B2 四个逻辑槽位。
- 验证玩家的槽位选择。
- 把玩家移动到对应 `PlayerStart`。
- 使用 AI 填充空槽位。
- 管理 AI 和玩家重生。
- 增加队伍比分。
- 检查 10 分胜利条件。
- 结束和重新开始比赛。

GameMode 不会复制到客户端，因此客户端不能直接依赖它显示 UI。

### `Variant_Shooter/EduShooterGameState.h/.cpp`

保存所有玩家都需要看到的比赛状态：

- 红队比分。
- 蓝队比分。
- 当前比赛模式。
- 比赛是否开始。
- 比赛是否结束。
- 获胜队伍。

服务器修改这些值后，Unreal 将它们复制到客户端。UI 再通过委托更新显示。

### `Variant_Shooter/EduShooterPlayerState.h/.cpp`

保存单个玩家的：

- 所属队伍。
- 已选择槽位。

这些信息放在 PlayerState 中，是因为 Pawn 死亡后 PlayerState 仍然存在，适合保存玩家身份。

## 6. 玩家控制与角色

### `Variant_Shooter/ShooterPlayerController.h/.cpp`

玩家与比赛系统之间的桥梁，负责：

- 添加输入 Mapping Context。
- 创建并管理本地 HUD。
- 显示模式选择界面。
- 显示队伍槽位选择界面。
- 显示比分和比赛结果。
- 向服务器发送模式、槽位和重开请求。
- 监听 Pawn 死亡并处理重生。
- 为本地死亡玩家创建和更新重生倒计时。
- 绑定复制后的 GameState。

典型请求流程：

```text
玩家点击 UMG 按钮
-> ShooterPlayerController
-> Server RPC
-> ShooterGameMode 验证请求
-> GameState 或 PlayerState 更新
-> 状态复制到客户端
-> PlayerController 更新本地 UI
```

### `Variant_Shooter/ShooterCharacter.h/.cpp`

玩家控制的战斗角色，负责：

- 当前血量和死亡状态。
- 武器库存和当前武器。
- 拾取和切换武器。
- 射击输入。
- 本地全自动射击请求节奏。
- 向服务器发送单发射击 RPC。
- 计算准星目标位置。
- 播放本地第一人称表现。
- 死亡和延迟重生。
- 复制预计重生的服务器时间。

客户端只请求开枪，最终是否允许射击由服务器决定。

## 7. 武器系统

### `Variant_Shooter/Weapons/ShooterWeaponHolder.h`

武器持有者接口。

玩家角色和 NPC 都实现该接口，因此 `ShooterWeapon` 不需要知道持有者究竟是玩家还是 AI。

接口包括：

- 挂接武器模型。
- 播放开火动画。
- 添加后坐力。
- 更新弹药 HUD。
- 取得瞄准位置。
- 添加和切换武器。

### `Variant_Shooter/Weapons/ShooterWeapon.h/.cpp`

武器的主要逻辑，负责：

- 第一和第三人称武器模型。
- 弹匣容量和当前弹药。
- 半自动或全自动模式。
- `RefireRate` 射速限制。
- 枪口位置。
- 瞄准偏差。
- 后坐力。
- 服务器生成 Projectile。
- 将弹药量复制给客户端。

`TryFireOnce()` 是服务器权威射击的关键入口，它会检查射速和弹药。

### `Variant_Shooter/Weapons/ShooterProjectile.h/.cpp`

实际飞行的子弹或榴弹，负责：

- 碰撞检测。
- Projectile Movement。
- 直接伤害。
- 爆炸范围伤害。
- 物理冲击。
- 命中特效广播。
- 命中后延迟销毁。

伤害应由服务器应用，客户端主要显示结果和特效。

### `Variant_Shooter/Weapons/ShooterPickup.h/.cpp`

关卡中的武器拾取物，负责：

- 检测角色进入拾取范围。
- 从 DataTable 读取显示模型和武器类。
- 把武器交给角色。
- 隐藏后等待定时重生。

## 8. AI 系统

### `Variant_Shooter/AI/ShooterNPC.h/.cpp`

AI 在世界中的身体，负责：

- 血量、伤害和死亡。
- 持有并使用武器。
- 瞄准当前目标。
- 开始和停止射击。
- 死亡表现和延迟销毁。

### `Variant_Shooter/AI/ShooterAIController.h/.cpp`

AI 的控制器和感知大脑，负责：

- 运行 StateTree。
- 使用 AI Perception 感知视觉和声音。
- 保存当前目标。
- 判断目标是否属于敌方队伍。
- 把感知事件传递给 StateTree Task。

### `Variant_Shooter/AI/ShooterStateTreeUtility.h/.cpp`

提供项目专用 StateTree 节点，例如：

- 感知敌人。
- 检查目标视线。
- 面向 Actor。
- 面向指定位置。
- 生成随机数。
- 向目标射击。

### `Variant_Shooter/AI/EnvQueryContext_Target.h/.cpp`

给 EQS 提供 AI 当前目标，使查询能够围绕目标寻找移动或射击位置。

### `Variant_Shooter/AI/ShooterNPCSpawner.h/.cpp`

原始模板刷怪器。

当前 2v2 槽位系统接管人口后会禁用这些刷怪器，避免产生额外 NPC。

## 9. UI 系统

### `EduMatchModeWidget.h/.cpp` 和 `WBP_MatchMode.uasset`

负责：

- 房主选择单人或双人模式。
- 客户端显示等待房主。
- 显示等待另一名玩家等状态。

C++ 处理行为，Widget Blueprint 处理布局、颜色和样式。

### `EduTeamSelectionWidget.h/.cpp` 和 `WBP_TeamSelection.uasset`

提供 R1、R2、B1、B2 四个槽位按钮，并把选择交给 PlayerController。

### `EduMatchResultWidget.h/.cpp` 和 `WBP_MatchResult.uasset`

显示 WIN/LOSE，并提供重新开始按钮。

### `ShooterUI.h/.cpp` 和 `UI_Shooter.uasset`

显示红蓝队伍比分。

### `ShooterBulletCounterUI.h/.cpp` 和 `UI_ShooterBulletCounter.uasset`

显示：

- 当前弹药量。
- 弹匣容量。
- 生命值。
- 受伤反馈。

### `EduRespawnCountdownWidget.h/.cpp` 和 `WBP_RespawnCountdown.uasset`

本地玩家死亡后显示重生剩余秒数：

- `ShooterCharacter` 在服务器记录预计重生时间并复制给客户端。
- `ShooterPlayerController` 使用同步后的服务器时间计算剩余秒数。
- `WBP_RespawnCountdown` 负责文本样式和可选动画。
- 新 Pawn 被控制或比赛结束后，倒计时控件会自动移除。

控件蓝图必须保留名为 `CountdownText` 的 `TextBlock`，否则 C++ 的 `BindWidget` 无法绑定。

### `BindWidget`

例如：

```cpp
UPROPERTY(meta=(BindWidget))
TObjectPtr<UButton> SinglePlayerButton;
```

这表示 Widget Blueprint 中必须存在同名控件 `SinglePlayerButton`。名称不匹配时，C++ 无法绑定该控件。

## 10. 关键 Blueprint 和内容资产

### 游戏框架 Blueprint

| 资产 | 作用 |
| --- | --- |
| `BP_ShooterGameMode` | `AShooterGameMode` 的可运行 Blueprint 子类 |
| `BP_ShooterPlayerController` | 配置角色类和各种 Widget Class |
| `BP_ShooterCharacter` | 配置玩家模型、动画、输入和默认属性 |
| `BP_ShooterNPC` | 配置 NPC 模型、武器和 AI Controller |
| `BP_ShooterAIController` | 配置感知和 StateTree |

### 武器 Blueprint

| 资产 | 作用 |
| --- | --- |
| `BP_ShooterWeaponBase` | 武器 Blueprint 基础配置 |
| `BP_ShooterWeapon_Pistol` | 手枪参数 |
| `BP_ShooterWeapon_Rifle` | 步枪参数 |
| `BP_ShooterWeapon_GrenadeLauncher` | 榴弹发射器参数 |
| `BP_ShooterProjectile_Bullet` | 普通子弹 |
| `BP_ShooterProjectile_Grenade` | 榴弹 |
| `DT_WeaponData` | 拾取物模型与武器类的对应数据 |

### AI 资产

| 资产 | 作用 |
| --- | --- |
| `ST_Shooter` | AI 总体行为 StateTree |
| `ST_Shooter_ShootAtTarget` | 射击目标相关子行为 |
| `EQS_FindRoamLocation` | 寻找巡逻位置 |
| `EQS_FindSnipingLocation` | 寻找射击位置 |

### 输入资产

| 资产 | 作用 |
| --- | --- |
| `IA_Shoot` | 射击动作 |
| `IA_SwapWeapon` | 切换武器动作 |
| `IMC_Weapons` | 把键鼠输入映射到武器动作 |

## 11. 关卡与配置

### `Content/Variant_Shooter/Lvl_Shooter.umap`

主 Shooter 关卡，包含：

- 世界场景。
- `PlayerStart`。
- NavMesh。
- 武器拾取物。
- AI 和其他 Actor。

红蓝双方需要各至少两个带对应 Tag 的 PlayerStart。

### `Config/DefaultEngine.ini`

当前重要配置：

- Editor 启动地图为 `Lvl_Shooter`。
- Game 默认地图为 `Lvl_Shooter`。
- 默认 GameMode 为 `BP_ShooterGameMode`。
- 定义 Projectile 碰撞通道。

### `Config/DefaultInput.ini`

启用 Enhanced Input。实际按键和动作关系主要配置在 `IA_*` 与 `IMC_*` 资产中。

## 12. 完整玩法数据流

### 模式和槽位选择

```text
WBP_MatchMode / WBP_TeamSelection
-> ShooterPlayerController
-> Server RPC
-> ShooterGameMode 验证
-> EduShooterGameState / EduShooterPlayerState
-> 状态复制
-> 各客户端 UI 更新
```

### 射击、伤害与计分

```text
IA_Shoot
-> ShooterCharacter::DoStartFiring
-> ShooterCharacter::ServerFireShot
-> ShooterWeapon::TryFireOnce
-> 服务器生成 ShooterProjectile
-> Projectile 命中
-> Character/NPC::TakeDamage
-> Die
-> ShooterGameMode::IncrementTeamScore
-> EduShooterGameState 复制比分
-> ShooterUI 更新
```

### 胜利与重开

```text
队伍达到 WinningScore
-> ShooterGameMode::FinishMatch
-> GameState 保存并复制胜者
-> PlayerController 显示 WIN/LOSE
-> 玩家点击 Restart
-> ServerRestartMatch RPC
-> GameMode 执行 Server Travel
```

## 13. 推荐学习顺序

1. 理解 GameMode、GameState、PlayerState 的区别。
2. 理解 PlayerController 与 Character 的区别。
3. 理解 `Server RPC`、`Replicated` 和 `OnRep`。
4. 跟踪一次完整射击、伤害和计分流程。
5. 理解 C++ 基类与 Blueprint 子类的分工。
6. 理解 AI Perception、StateTree 和 EQS 的协作。

掌握以上部分后，就能从“会运行这个项目”进入“理解为什么它能运行”的阶段。
