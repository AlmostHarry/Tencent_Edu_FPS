# Tencent Edu FPS

基于 Unreal Engine 5.7 开发的第一人称射击课程项目，面向腾讯游戏「开局一课」客户端方向。

项目实现了一个可在 UE PIE 中运行的服务器权威 2v2 团队对战 MVP，支持单人模式和双人 Listen Server 模式。红蓝双方各包含两个逻辑槽位（`R1`、`R2`、`B1`、`B2`），未被真人玩家选择的槽位由 AI 补齐。

## 功能

- 第一人称移动、瞄准、射击、武器拾取与切换
- 追踪和攻击敌方角色的 AI
- 红蓝队伍、固定出生槽位与 AI 自动补位
- 同队伤害拦截和敌我识别
- 服务器权威的射击、弹药、伤害、死亡和计分
- 玩家与 AI 死亡后在原槽位重生
- 队伍比分、个人 KDA 和 Tab 计分板
- 首支获得 5 分的队伍获胜
- 胜负结算与服务器同步重新开始
- 单人及双人 Listen Server PIE 支持

## 环境要求

- Unreal Engine 5.7
- Windows 64-bit
- Visual Studio 2022，以及 Unreal Engine C++ 开发所需组件

项目默认关卡为：

```text
/Game/Variant_Shooter/Lvl_Shooter
```

## 运行项目

1. 克隆仓库。
2. 使用 Unreal Engine 5.7 打开 `Tencent_Edu_FPS.uproject`。
3. 如果编辑器提示模块需要重新编译，允许其完成编译。
4. 确认打开 `/Game/Variant_Shooter/Lvl_Shooter`。
5. 点击 Play 运行。

也可以在关闭编辑器后使用以下命令构建 Editor Target：

```powershell
<UE_ROOT>\Engine\Build\BatchFiles\Build.bat Tencent_Edu_FPSEditor Win64 Development -Project="<PROJECT_ROOT>\Tencent_Edu_FPS.uproject" -WaitMutex
```

其中 `<UE_ROOT>` 为 Unreal Engine 5.7 安装目录，`<PROJECT_ROOT>` 为本项目根目录。

## 基本操作

| 操作 | 输入 |
| --- | --- |
| 移动 | `W` `A` `S` `D` |
| 视角 | 鼠标 |
| 跳跃 | `Space` |
| 射击 | 鼠标左键 |
| 查看计分板 | 按住 `Tab` |

进入比赛前，Listen Server 主机选择单人或双人模式。随后每位真人玩家选择一个未被占用的 `R1`、`R2`、`B1` 或 `B2` 槽位。

## 网络架构

项目使用 Unreal Engine 状态复制和 Listen Server，不采用确定性帧同步。

- `GameMode`：服务器侧比赛规则、槽位管理、AI 补位、计分和胜负判定
- `GameState`：复制比赛模式、队伍比分、计分板和胜者
- `PlayerState`：复制玩家队伍槽位和个人比赛数据
- `PlayerController`：本地输入、UI 和服务器请求
- `Character` / `Weapon` / `Projectile`：服务器权威的射击、伤害和角色状态

客户端负责输入、HUD、相机和表现；服务器负责验证射击并处理弹药、伤害、死亡、重生、计分和胜利。

## 目录说明

```text
Config/                         项目、地图、渲染和输入配置
Content/                        关卡、Blueprint、Widget、材质和其他 UE 资产
Docs/                           实现说明和回归测试文档
Source/Tencent_Edu_FPS/         C++ 游戏模块
  Variant_Shooter/              射击玩法、队伍、网络、AI、武器和 UI 逻辑
Tencent_Edu_FPS.uproject        Unreal Engine 项目入口
```

## 当前状态

当前版本已实现完整的单人和双人 Listen Server 对战流程，包括射击、伤害、死亡、重生、计分、胜利和比赛重新开始。
