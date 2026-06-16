# JumpPad AI 固定方向弹射蓝图操作指南

这份文档只用于手动修改蓝图，不需要提交到 Git。

## 目标

保持玩家踩 JumpPad 的现有行为不变，只让 AI 踩 JumpPad 时按照每个 JumpPad 实例配置的固定方向弹射。

AI 弹射参数拆成三部分：

```text
AI_LaunchDirection  // 只控制水平朝向
AIHorizontalSpeed   // 控制水平飞行距离
AIVerticalSpeed     // 控制向上弹射高度
```

这样水平速度和垂直速度不会互相影响。旋转箭头只决定 AI 往哪个水平方向飞，飞多远和飞多高分别用两个 Float 变量调。

实现仍然放在现有的 `BP_JumpPad` 蓝图里，不需要新建 C++ 类。

## 打开蓝图

1. 打开 Unreal Editor。
2. 在内容浏览器搜索 `BP_JumpPad`。
3. 打开 `/Game/LevelPrototyping/Interactable/JumpPad/BP_JumpPad`。
4. 进入 `Event Graph`。

当前蓝图流程大致是：

```text
ActorBeginOverlap
-> Actor Has Tag
-> Branch
-> Cast To Character
-> Launch Character
```

不要修改现有玩家用的 `Launch Character` 节点参数。

## 添加 AI 弹射方向箭头

1. 在左上角 `Components` 面板点击 `+ Add`。
2. 搜索 `Arrow`。
3. 添加一个 `Arrow Component`。
4. 重命名为 `AI_LaunchDirection`。

这个箭头只用来控制 AI 的水平弹射方向。回到关卡后，每个 JumpPad 实例都可以单独旋转这个箭头。

注意：这个方案会在蓝图里忽略箭头的 Z 分量，所以你不需要调整箭头的俯仰角。只需要调整左右朝向，也就是 `Yaw`。

## 添加 AI 速度变量

1. 在左侧 `Variables` 面板点击 `+`。
2. 新建变量 `AIHorizontalSpeed`。
3. 类型设置为 `Float`。
4. 勾选 `Instance Editable`。
5. 默认值可以先设置为 `1200` 或 `1500`。
6. 再新建变量 `AIVerticalSpeed`。
7. 类型设置为 `Float`。
8. 勾选 `Instance Editable`。
9. 默认值可以先设置为 `900` 或 `1000`。
10. 点击一次 `Compile`，这样默认值才会正常显示和可编辑。

如果你之前已经创建了 `AILaunchSpeed`，可以删掉它，或者暂时留着不用。新的方案不再使用一个总速度。

## 区分玩家和 AI 分支

在现有 `Cast To Character` 节点后面插入判断：

1. 从 `As Character` 引脚拉线。
2. 搜索 `Is Player Controlled`。
3. 添加一个新的 `Branch`。
4. 连接方式：

```text
Cast To Character 成功执行线 -> 新 Branch 执行输入
Is Player Controlled 返回值 -> 新 Branch 的 Condition
```

然后连接：

```text
Branch True  -> 原来的 Launch Character
Branch False -> 新建的 AI 专用 Launch Character
```

重点：原来的 `Launch Character` 就作为玩家分支继续使用，里面的 `Launch Velocity`、`XYOverride`、`ZOverride` 都保持原样，不要改。

## 构造 AI 弹射速度

给 AI 分支新建第二个 `Launch Character` 节点。

AI 的 `Launch Velocity` 按下面方式构造：

```text
水平速度 = AI_LaunchDirection.ForwardVector 去掉 Z 后归一化 * AIHorizontalSpeed
垂直速度 = AIVerticalSpeed

AI Launch Velocity = (水平速度.X, 水平速度.Y, AIVerticalSpeed)
```

具体蓝图操作：

1. 把 `AI_LaunchDirection` 拖到图表里，选择 `Get`。
2. 从它拉线，调用 `Get Forward Vector`。
3. 从 `Get Forward Vector` 的返回值拉线，创建 `Break Vector`。
4. 新建一个 `Make Vector`，用于重新组合水平朝向：

```text
X = ForwardVector.X
Y = ForwardVector.Y
Z = 0
```

5. 从这个 `Make Vector` 的返回值拉线，搜索 `Normalize`。
6. 把 `AIHorizontalSpeed` 拖到图表里，选择 `Get`。
7. 创建乘法节点：

```text
Normalize 后的水平 Vector * AIHorizontalSpeed
```

如果搜索不到 `Vector * Float`，就搜索：

```text
Multiply
```

或者中文环境里的：

```text
乘法
```

8. 对这个乘法结果使用 `Break Vector`。
9. 把 `AIVerticalSpeed` 拖到图表里，选择 `Get`。
10. 新建最终的 `Make Vector`：

```text
X = 水平速度的 X
Y = 水平速度的 Y
Z = AIVerticalSpeed
```

11. 把最终 `Make Vector` 连接到 AI 专用 `Launch Character` 的 `Launch Velocity`。
12. 把 `As Character` 连接到 AI 专用 `Launch Character` 的 `Target`。
13. AI 专用 `Launch Character` 的勾选项设置为：

```text
XYOverride = true
ZOverride = true
```

AI 分支需要 `XYOverride = true`，否则 AI 原本的寻路移动速度会叠加进去，弹射方向会偏。

## 编译并配置关卡中的实例

1. 点击 `Compile`。
2. 点击 `Save`。
3. 回到关卡。
4. 选中每个 JumpPad 实例。
5. 旋转 `AI_LaunchDirection` 箭头的 `Yaw`，让它指向 AI 应该飞出去的水平面方向。
6. 如果某个 JumpPad 需要飞得更远或更近，可以单独调整该实例的 `AIHorizontalSpeed`。
7. 如果某个 JumpPad 需要飞得更高或更低，可以单独调整该实例的 `AIVerticalSpeed`。

调试建议：

```text
AIHorizontalSpeed = 1200
AIVerticalSpeed   = 1000
```

如果 AI 飞不到目标平台：

```text
先提高 AIHorizontalSpeed，让它飞得更远。
如果高度不够，再提高 AIVerticalSpeed。
```

如果 AI 飞过头：

```text
先降低 AIHorizontalSpeed。
如果高度太夸张，再降低 AIVerticalSpeed。
```

## 可选：AI 冷却

如果 AI 仍然落回触发区域并反复弹起，再考虑加 AI 专用冷却：

```text
AI 触发 JumpPad
-> 如果不在冷却中
-> 弹射 AI
-> 把该 AI 加入冷却集合或 Map
-> 0.5 秒后从冷却中移除
```

只有当固定方向弹射还不能解决重复弹起时，再加这一步。
