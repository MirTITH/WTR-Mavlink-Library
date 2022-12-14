# WTR Mavlink Library

WTRobot, HITsz

## 介绍

移植到 stm32 平台的 mavlink

MAVLink 是一套消息传输协议。可以使用它方便地在单片机之间传输信息（变量、结构体等），并且有校验机制，可以检测并丢弃传输错误的包。

本库支持 MavLink V1 和 V2 版本的协议。

MAVLink 官网: <https://mavlink.io/zh/>

## 安装 mavlink

### 获取 mavlink

方法一：克隆或下载本仓库，里面的 `mavlink_1.0.12` 文件夹就是 1.0.12 版本的 mavlink 生成器和 2.4.36 版本的 pymavlink

方法二：从官方库中下载：

1. clone 官方库
    ```
    git clone https://github.com/mavlink/mavlink.git --recursive
    ```
2. 切换到 release 版本
    ```sh
    cd mavlink
    # 查看有哪些版本的 mavlink 库
    git tag
    # 切换到你希望的版本，例如：
    git checkout 1.0.12 

    cd pymavlink
    # 查看有哪些版本的 pymavlink 库
    git tag
    # 切换到你希望的版本。例如：
    git checkout 2.4.36
    ```

### 安装环境

使用 mavlink 生成消息前，需要安装 python 和一些模块

> 安装参考 <https://mavlink.io/zh/getting_started/installation.html>  
> 上面的链接中，Windows 下只需要装好 python 和 future 模块就行了，Linux 下可能还要安装 TkInter

## 开始上手

### 生成自定义消息

消息就是你要发送或接收的一组变量。

消息在 xml 文件中定义。mavlink 会根据 xml 文件生成对应的结构体以及收发函数。

如果想在两个系统之间传输消息，那么这个消息必须在这两个系统中都定义（而且这个消息的id和内容要相同）

#### 编写 `.xml` 文件

可以仿照以下文件编写 `.xml`：

[wtr_mavlink_demo.xml](examples/stm32f103cbt6/UserCode/mavlink/wtr_mavlink_demo.xml)

[test.xml](mavlink_1.0.12/message_definitions/v1.0/test.xml)

> 建议将你写的 xml 文件也放在你的工程目录中，以免以后修改时找不到之前写的文件  

以下是 MavLink 官方关于如何选取消息 id 的建议：

> 对于 MAVLink 1:  
> - 有效数字介于 0 到 255。
> - ID 0-149 和 230-255 为common.xml保留。 语支可以使用180-229 用于自定义消息 (除非这些信息没有被其他包括语支使用)。
> 
> 对于 MAVLink 2 :
> - 有效数字介于0-1677215。
> - 255以下所有值都被认为是保留的，除非报文也打算用于 MAVLink 1。 **注意** ID 在 MAVLink 1 中很宝贵！

> mavlink 官方有一些预定义好的消息（放在这里 [mavlink_1.0.12/message_definitions/v1.0](mavlink_1.0.12/message_definitions/v1.0)）。这些消息是官方预设的无人机通信消息，如果不玩无人机就基本上用不到。

详细语法见官方文档: 

<https://mavlink.io/zh/guide/xml_schema.html>

<https://mavlink.io/zh/guide/define_xml_element.html>

#### 根据 xml 文件生成库文件

1. 打开 `mavlink/mavgenerate.py` 
   - XML 选择你刚刚编写的 XML 文件
   - Out 选择你想要存放库文件的目录
   - Language 选择 C
   - Protocol 选择 1.0 或 2.0（看你需求，版本区别官网上有说明）
2. Generate

### 移植到 stm32

1. 将生成的库文件添加到你的 stm32 工程中
2. 将 `src/wtr_mavlink.c` 和 `src/wtr_mavlink.h` 添加到你的 stm32 工程中
3. 对于 MDK，可能要开启 GNU 拓展
4. 根据实际需要，修改 `wtr_mavlink.h` 中的以下内容
    ```
    // 系统 ID 和 组件 ID
    static mavlink_system_t mavlink_system = {
        1, // System ID (1-255)
        1  // Component ID (1-255)
    };

    // MavLink 能使用的最大通道数
    #define MAVLINK_COMM_NUM_BUFFERS 4
    ```

    MAVLink 支持多个系统互相通信。系统可以是上位机、机器人、遥控器、无人机等。一般建议为每个系统分配唯一的 `System ID`。如果一个系统上有多个单片机，建议为每个单片机分配唯一的 `Component ID`。

    > 这两个 ID 只是为了让接收方知道消息是谁发过来的。
    > MAVLink 原本设想的是为系统上的每个组件（比如说各种传感器）分配一个 `Component ID`，但这里的 mavlink_system 是全局变量，一个单片机不方便设置多个 `Component ID`，所以只能每个单片机分配一个 `Component ID` 了。

    > `MAVLINK_COMM_NUM_BUFFERS` 是 MAVLink 最大可用的通道数。通常一个通道对应一个串口。请根据需要分配。设置得越大会占用越多内存。MAVLink 默认为单片机分配 4 个通道。

## 使用 WTR Mavlink Library

使用 mavlink 相关函数前，需要 `#include "wtr_mavlink.h"`

> 只需要包含 `wtr_mavlink.h` 这一个文件就行了，不用包含 mavlink 的其他头文件

### 绑定通道和串口

> mavlink 支持多通道收发，使用 mavlink 的通道前需要先绑定串口，之后对这个通道的发送和接收操作就相当于对绑定的串口的操作（串口要在 CubeMX 里先配置好）

> 一般一个通道对应一个串口，一个通道可以同时收发，通道的数量取决于你在`wtr_mavlink.h`中定义的 `MAVLINK_COMM_NUM_BUFFERS`

使用 `wtrMavlink_BindChannel()` 函数绑定

examples:

```c
wtrMavlink_BindChannel(&huart1, MAVLINK_COMM_0);
wtrMavlink_BindChannel(&huart2, MAVLINK_COMM_1);
```

### 发送消息

mavlink 将你自定义的消息都封装在了结构体里，一个消息对应一个结构体

如名字为 `speed` 的消息在 `mavlink_speed_t` 中

本库采用阻塞式发送，使用如下函数发送结构体（记得绑定通道和串口）：

```c
// 向通道X发送结构体，（X要改为对应的数字）
mavlink_msg_xxx_send_struct(MAVLINK_COMM_X, &StructToBeSend);
```

> xxx 为要发送的消息名称

### 接收消息

本库采用中断接收模式，因此如果需要接收消息，必须在 CubeMX 里使能串口全局中断

接收消息需要如下操作：

1. 确保已经绑定通道和串口

2. 在代码中调用：
    ```c
    // 启动通道X对应串口的中断接收
    wtrMavlink_StartReceiveIT(MAVLINK_COMM_X);
    ```

3. 在中断回调函数中调用`wtrMavlink_UARTRxCpltCallback()`

    例如:
    ```c
    void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
    {
        // 接收通道X的消息
        wtrMavlink_UARTRxCpltCallback(huart, MAVLINK_COMM_X);
    }
   ```

4. 在你喜欢的位置（如 main.c）定义如下函数：

    ```c
    /**
     * @brief 接收到完整消息且校验通过后会调用这个函数。在这个函数里调用解码函数就可以向结构体写入收到的消息
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

    > 以上代码通过 `msg->msgid` 判断是哪个消息，还可以通过 `msg->sysid` 和 `msg->compid` 判断消息是从哪里来的

## 更多 example

更多 example 可以参考 [examples](examples/) 下的工程

## 性能测试

stm32f103 在 arm-none-eabi-gcc-10.3.1 编译器下：

开启 O3 优化：2M 波特率成功收发，2.5M 波特率接收失败

不开优化：1M 波特率接收失败，115200 (0.1M) 波特率接收成功

## 作者
X. Y.

- github: [MirTITH (github.com)](https://github.com/MirTITH)
- gitee: [TITHChan (tithchan) - Gitee.com](https://gitee.com/tithchan)

## 版本日志

### 1.2
- 增加数组下标溢出判断
- 修改文件注释

### 1.1.1
- 更新 readme
- 添加 .gitignore
- 更新 pymavlink 脚本到 2.4.36 版本
- 发现这个库直接支持 V2 协议（原本是在 V1 协议 mavlink 库上设计的）（😀）

### 1.1
- 修复同时收发时可能的卡串口问题

### 1.0
- 优化性能
- 重构 API
- 使用外部 mavlink_get_channel_status 和 mavlink_get_channel_buffer
- 完善注释

### 0.2
上古代码，赶时间写出来的，有许多问题，和之后的版本差别巨大，不建议使用
