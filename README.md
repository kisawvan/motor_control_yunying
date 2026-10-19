# motor_control_yunying

基于 STM32F103C8、标准外设库与 FreeRTOS 的 CAN 电机控制实验工程，包含按键输入、报文生成和重力补偿计算。

## 已有代码

- CAN1 初始化、标准帧发送、轮询接收与发送超时处理。
- 电机位置报文模板，以及速度、力矩、角度到 16 位字段的示例映射。
- 杆件与末端负载的重力力矩计算。
- 按键驱动、OLED 与 LED 驱动文件。
- FreeRTOS 启动任务和两个周期打印任务。

**当前集成状态：** `main()` 在 CAN、按键初始化后调用 `FreeRTOS_Start()`。调度器正常运行时不会返回，因此后面的电机发送和按键主循环不会按顺序执行。当前两个 RTOS 任务只做打印，尚未承担电机控制；生成重力补偿报文后的发送语句也处于注释状态。

## 目录导航

| 路径 | 用途 |
| --- | --- |
| [Project.uvprojx](Project.uvprojx) | Keil 工程，目标 STM32F103C8 |
| [User/main.c](User/main.c) | 电机参数、重力补偿、报文生成与入口 |
| [User/freertos_demo.c](User/freertos_demo.c) | 调度器启动和示例任务 |
| [Hardware/MyCAN.c](Hardware/MyCAN.c) | CAN 初始化和收发 |
| [Hardware/Key.c](Hardware/Key.c) | PB1、PB11、PA15 按键读取 |
| [freertos/FreeRTOSConfig.h](freertos/FreeRTOSConfig.h) | RTOS 配置 |
| [System/](System/) / [Start/](Start/) / [Library/](Library/) | 延时、启动文件与标准外设库 |
| [LICENSE](LICENSE) | 仓库许可证；第三方组件同时遵循其自身声明 |

## 构建与调试准备

1. 用 Keil MDK 打开 `Project.uvprojx`。工程记录的芯片包为 `Keil.STM32F1xx_DFP.2.2.0`。
2. 核对板卡芯片、时钟、下载器和启动文件。FreeRTOS 配置使用 72 MHz CPU 时钟、1000 Hz tick 和 17 KiB 动态堆；需检查最终 RAM 占用。
3. CAN1 使用 PA11（RX）与 PA12（TX）。源码时序为 Prescaler=3、BS1=8 tq、BS2=3 tq，在 CAN 外设时钟 36 MHz 时对应 1 Mbit/s。
4. 按实际收发器和电机驱动器完成接线，并核对总线速率、节点 ID 和协议。
5. 将电机发送、按键或反馈处理接入 RTOS 任务后，再做功能联调。当前不能把下载该工程等同于启动了电机控制任务。

## 电机报文与模型边界

`generate_motor_command()` 以 8 字节模板为基础，按大端写入：字节 2–3 为速度，4–5 为力矩，6–7 为位置（下标从 0 开始）。`TxID` 默认为 `0x01`。

| 参数 | 默认值 | 当前含义 |
| --- | --- | --- |
| `MAX_TORQUE_NM` | 5.0 N·m | 示例映射上限 |
| `MAX_SPEED_RPM` | 6000 rpm | 示例映射上限 |
| 角度映射 | 360° → 65535 | 示例单圈映射 |
| 杆长 / 杆质量 | 1.00 m / 0.540 kg | 重力补偿模型参数 |
| 负载 / 力臂 | 0.2788 kg / 1.00 m | 重力补偿模型参数 |

上述单位映射是源码中的假设，不能代替具体驱动器协议。补偿函数返回力矩绝对值，未处理带符号方向；它使用目标角度计算，尚未接入反馈角度形成闭环。静态模板和生成函数的字节解释也需按设备手册逐项核对。

`can_send_data()` 成功后会清空调用者的发送缓存；`MyCAN_Transmit()` 在等待结束后也会清空缓存，重复使用模板时需特别留意。

## 后续工作

- 将控制逻辑迁入 RTOS 任务，完善日志输出通道和任务间数据传递。
- 明确电机型号、协议、字节序、量程与反馈解析。
- 校准模型参数与补偿方向，再记录空载及带载测试。
- 补充编译结果、接线图和实机验证记录。

本文基于仓库代码整理，未重新编译或进行电机实测。
