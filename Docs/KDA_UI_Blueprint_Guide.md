# KDA UI 蓝图配置指南

## 目的

本文档说明当前 C++ 已经完成的 KDA 功能，以及还需要在 Unreal Editor 中手动完成的
Widget Blueprint 配置。

当前实现遵循项目约定的 C++ Parent + Presentation Blueprint 工作流：

1. C++ 负责战绩数据、复制、校验、更新事件和 Blueprint 可调用接口。
2. Widget Blueprint 负责布局、样式、动画和可调展示效果。
3. 最终需要把 C++ 文件和必需的 `.uasset` 一起提交，保证干净 checkout 后项目可运行。

## C++ 已完成内容

本地玩家的 KDA 数据存放在 `AEduShooterPlayerState`。

已实现字段：

- `Kills`
- `Deaths`
- `Assists`

已实现计分规则：

- 玩家死亡时，该玩家的 `Deaths` 加 1。
- 最后一击造成死亡的玩家，`Kills` 加 1。
- 目标本次生命期间，其他曾经造成有效 HP 减少的玩家，`Assists` 加 1。
- 友军伤害、0 伤害、负数伤害、自伤、目标死亡后的伤害都不会产生助攻记录。

已实现 UI 连接逻辑：

- `UEduKDAWidget` 是 KDA HUD 的 C++ 父类。
- `AShooterPlayerController` 会为每个本地玩家创建 `/Game/Variant_Shooter/UI/WBP_KDA`。
- 该 Widget 会以 Z-order `50` 添加到本地玩家屏幕。
- 拥有者 PlayerState 的 KDA 复制更新后，Widget 会自动刷新。
- 结算界面也会收到最终 KDA，可显示 `Your KDA: X/X/X`。

## 必需的 HUD Widget Blueprint

需要新建这个资产：

`/Game/Variant_Shooter/UI/WBP_KDA`

父类必须设置为：

`EduKDAWidget`

Widget Tree 建议结构：

- Root：`Canvas Panel`
- 子控件：`TextBlock`
  - Name：可以保留默认的 `TextBlock_0`，也可以使用任何不冲突的名字
  - 勾选 `Is Variable`
  - 初始 Text 可填：`K/D/A 0/0/0`

C++ 父类不再使用 `BindWidget` 绑定 HUD 文本控件。这样可以避免父类 C++ 成员名和 Widget Designer 控件名互相占用的问题。

## HUD 布局建议

把 KDA 文本放在左上角血条下方。

以 1920x1080 视口为例，建议值：

- Anchor：左上角
- Position X：`66`
- Position Y：`118`
- Size X：`220`
- Size Y：`32`
- Font Size：`18`
- Color：白色
- Shadow Offset：`1, 1`
- Shadow Color：黑色，Alpha 约 `0.75`

如果 PIE 时和血条重叠，就稍微往下移动。

## 必需蓝图事件

`EduKDAWidget` 暴露了这个 Blueprint 事件：

`KDA Updated`

输入：

- `Kills`
- `Deaths`
- `Assists`
- `DisplayText`

需要在 `WBP_KDA` 里实现这个事件：

1. 进入 `Graph`。
2. 添加或覆写事件 `KDA Updated`。
3. 把你的 TextBlock 变量拖进图表，例如 `TextBlock_0`，选择 `Get`。
4. 从 TextBlock 引脚拉出，调用 `SetText`。
5. 把 `KDA Updated` 的 `DisplayText` 引脚接到 `SetText` 的文本输入。

也可以直接用 `Kills`、`Deaths`、`Assists` 三个数值自己拼文本，但默认推荐直接使用 `DisplayText`。

## 结算界面蓝图修改

打开已有资产：

`/Game/Variant_Shooter/UI/WBP_MatchResult`

添加一个用于最终 KDA 的 `TextBlock`：

- Name：可以保留默认名，或使用任何不冲突的名字
- 勾选 `Is Variable`
- 初始 Text：`Your KDA: 0/0/0`

建议位置：

- 居中放在现有 `WIN` / `LOSE` 文本下方
- 放在 Restart 按钮上方
- Font Size：`24`
- Color：白色
- Shadow Offset：`1, 1`
- Shadow Color：黑色，Alpha 约 `0.75`

`EduMatchResultWidget` 也不再使用 `BindWidget` 绑定最终 KDA 文本。最终 KDA 通过蓝图事件传入。

需要实现 Blueprint 事件：

`Player KDA Updated`

输入：

- `Kills`
- `Deaths`
- `Assists`
- `DisplayText`

在 `WBP_MatchResult` 中实现方式和 HUD 类似：

1. 进入 `Graph`。
2. 添加或覆写事件 `Player KDA Updated`。
3. 把最终 KDA 的 TextBlock 变量拖进图表，选择 `Get`。
4. 调用 `SetText`。
5. 把 `DisplayText` 接到 `SetText`。

## 验证流程

创建并保存 `WBP_KDA` 后：

1. 关闭 Unreal Editor。
2. 构建 Editor 目标：

```powershell
E:\UnrealEngine\UE_5.7\Engine\Build\BatchFiles\Build.bat Tencent_Edu_FPSEditor Win64 Development -Project="E:\Projects\UEProjects\Tencent_Edu_FPS\Tencent_Edu_FPS.uproject" -WaitMutex
```

3. 打开项目。
4. 运行 Shooter 地图。
5. 确认血条下方出现 `K/D/A 0/0/0`。
6. 击杀一个敌人，确认本地玩家的 Kill 数增加。
7. 在 2 人 PIE 中，让两个玩家先后伤害同一个目标，再由其中一个玩家击杀。最后击杀者应获得 `K`，另一个造成过伤害的玩家应获得 `A`。
8. 比赛结束后，确认结算 UI 显示 `Your KDA: X/X/X`。

## 提交说明

功能验证通过后，KDA UI 相关提交应包含：

- `Content/Variant_Shooter/UI/WBP_KDA.uasset`
- `Content/Variant_Shooter/UI/WBP_MatchResult.uasset`
- KDA 相关 C++ 源文件
- `Docs/KDA_UI_Blueprint_Guide.md`

不要提交 `Content/XL_FPSPack/`，除非项目后续确实开始依赖这个素材包。
