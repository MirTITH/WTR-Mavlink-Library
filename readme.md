# WTR Mavlink Library

WTRobot, HITsz

## 介绍

移植到 stm32 平台的 mavlink

MAVLink 支持自定义消息，因此可以用来通过串口传输结构体，并且自带校验。

本库针对 MavLink V1 版本编写。

MAVLink 官网: <https://mavlink.io/zh/>

## 开始上手

### 生成带有自定义消息的 mavlink 库文件

#### 编写 `.xml` 文件

注意：消息 ID 介于 0 到 255 之间

可以参考以下文件：

[wtr_mavlink_demo.xml](mavlink_generator/message_definitions/v1.0/wtr_mavlink_demo.xml)

[test.xml](mavlink_generator/message_definitions/v1.0/test.xml)

详细介绍见官方文档: 

<https://mavlink.io/zh/guide/xml_schema.html>

<https://mavlink.io/zh/guide/define_xml_element.html>

#### 生成库文件

> 需先安装 python 和一些模块  
> 安装参考 <https://mavlink.io/zh/getting_started/installation.html>  
> 上面的链接中，Windows 下只需要装好 python 和 future 模块就行了，Linux 下可能还要安装 TkInter

1. 打开 `mavlink_generator/mavgenerate.py` 
   - XML 选择你刚刚编写的 XML 文件
   - Out 选择你想要存放库文件的目录
   - Language 选择 C
   - Protocol 选择 1.0
2. Generate

### 移植到 stm32

1. 将生成的库文件添加到你的 stm32 工程中
2. 将 `src/wtr_mavlink.c` 和 `src/wtr_mavlink.h` 添加到你的 stm32 工程中
3. 根据实际需要，修改 `wtr_mavlink.h` 中的以下内容
    ```
    // 系统 ID 和 组件 ID
    static mavlink_system_t mavlink_system = {
        1, // System ID (1-255)
        1  // Component ID (1-255)
    };

    // MavLink 能使用的最大通道数
    #define MAVLINK_COMM_NUM_BUFFERS 4
    ```

    MAVLink 支持多个系统互相通信。系统可以是机器人、遥控器、无人机等。一般建议为每个系统分配唯一的 `System ID`。如果一个系统上有多个单片机，建议为每个单片机分配唯一的 `Component ID`。

    > 这两个 ID 只是为了区分收到的消息来自哪里，如果不需要区分，可以都取相同 ID。（但不太建议）  
    > MAVLink 原本设想的是为系统上的每个组件（比如说各种传感器）分配一个 `Component ID`，但这里的 mavlink_system 是全局变量，一个单片机不方便设置多个 `Component ID`，所以只能每个单片机分配一个 `Component ID` 了。

    > `MAVLINK_COMM_NUM_BUFFERS` 是 MAVLink 最大可用的通道数。通常一个通道对应一个串口。请根据需要分配。设置得越大会占用越多内存。MAVLink 默认为单片机使用 4 个通道。

## 使用 WTR Mavlink Library

### 绑定通道和串口

examples:

```c
wtrMavlink_BindChannel(&huart1, MAVLINK_COMM_0);
wtrMavlink_BindChannel(&huart2, MAVLINK_COMM_1);
```

### 发送结构体

examples:

```
mavlink_msg_xxx_send_struct(MAVLINK_COMM_1, &StructToBeSend);
```

> xxx 为要发送的消息名称（消息都是封装在结构体里的）

### 接收结构体

本库采用中断接收模式，所以必须打开串口全局中断

examples:

在 main.c 中：
```c
// 开启通道0的接收中断
wtrMavlink_StartReceiveIT(MAVLINK_COMM_0);
```

在中断回调函数中：
```c
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    // 接收通道0的消息
    wtrMavlink_UARTRxCpltCallback(huart, MAVLINK_COMM_0);
}
```

在你喜欢的位置（如 main.c）定义如下函数：

> 你也可以通过 `msg->sysid` 和 `msg->compid` 判断消息是从哪里来的

```c
/**
 * @brief 接收到完整消息且校验通过后会调用这个函数。在这个函数里调用解码函数就可以向结构体写入收到的数据
 *
 * @param msg 接收到的消息
 * @return
 */
void wtrMavlink_MsgRxCpltCallback(mavlink_message_t *msg)
{
    switch (msg->msgid) {
        case 1:
            // id = 1 的消息对应的解码函数(mavlink_msg_xxx_decode)
            mavlink_msg_xxx_decode(msg, &StructReceived);
            break;
        case 2:
            // id = 2 的消息对应的解码函数(mavlink_msg_xxx_decode)
            break;
        // ......
        default:
            break;
    }
}
```

## 更多 example

更多 example 可以参考 [examples](examples/) 下的工程

## 性能测试

stm32f103 在 arm-none-eabi-gcc-10.3.1 编译器下：

开启 O3 优化：2M 波特率成功收发，2.5M 波特率接收失败

不开优化：1M 波特率接收失败，115200 (0.1 M) 波特率接收成功

## 作者
X. Y.

- github: [MirTITH (github.com)](https://github.com/MirTITH)
- gitee: [TITHChan (tithchan) - Gitee.com](https://gitee.com/tithchan)

## 版本日志

### 1.1
- 修复同时收发时可能的卡串口问题

### 1.0
- 优化性能
- 重构 API
- 使用外部 mavlink_get_channel_status 和 mavlink_get_channel_buffer
- 完善注释

### 0.2
上古代码，赶时间写出来的，有许多问题，和之后的版本差别巨大，不建议使用
