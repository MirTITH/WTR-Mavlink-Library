#include "user_main.h"
#include "main.h"
#include "wtr_mavlink.h"
#include "stdio.h"

mavlink_gyro_t gyroStructToBeSend, gyroStructReceived;
mavlink_speed_t speedStructToBeSend, speedStructReceived;

void user_main(void)
{
    // 绑定串口和通道
    wtrMavlink_BindChannel(&huart1, MAVLINK_COMM_0);
    wtrMavlink_BindChannel(&huart2, MAVLINK_COMM_1);

    // 开启通道0的接收中断
    wtrMavlink_StartReceiveIT(MAVLINK_COMM_0);

    for (;;) {
        // 模拟变化量
        gyroStructToBeSend.ax += 1;
        gyroStructToBeSend.ay += 2;
        gyroStructToBeSend.az += -2;
        speedStructToBeSend.rot += -1;
        speedStructToBeSend.vx += 0.5;
        speedStructToBeSend.vy += -0.5;

        // 向通道1发送结构体
        mavlink_msg_gyro_send_struct(MAVLINK_COMM_1, &gyroStructToBeSend);
        mavlink_msg_speed_send_struct(MAVLINK_COMM_1, &speedStructToBeSend);
        HAL_Delay(100);
    }
}

void wtrMavlink_MsgRxCpltCallback(mavlink_message_t *msg)
{
    switch (msg->msgid) {
        case MAVLINK_MSG_ID_SPEED:
            // 将接收到的消息解码
            mavlink_msg_speed_decode(msg, &speedStructReceived);
            break;
        case MAVLINK_MSG_ID_GYRO:
            // 将接收到的消息解码
            mavlink_msg_gyro_decode(msg, &gyroStructReceived);
            break;
        // ......
        default:
            break;
    }
}