/**
 * @file wtr_mavlink.c
 * @author X. Y.
 * @brief 移植到 stm32 的 mavlink（只需要包含 wtr_mavlink.h 就够了）
 * @version 1.0
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "wtr_mavlink.h"

wtrMavlink_handle_t hMAVLink[MAVLINK_COMM_NUM_BUFFERS] = {0};

void wtrMavlink_BindChannel(UART_HandleTypeDef *huart, mavlink_channel_t chan)
{
    hMAVLink[chan].huart = huart;
}

int wtrMavlink_StartReceiveIT(mavlink_channel_t chan)
{
    if (hMAVLink[chan].huart == NULL) {
        return -1;
    }

    return HAL_UART_Receive_IT(hMAVLink[chan].huart, &(hMAVLink[chan].rx_buffer), 1);
}

__weak void wtrMavlink_MsgRxCpltCallback(mavlink_message_t *msg)
{
    switch (msg->msgid) {
        case 1:
            // id = 1 的消息对应的解码函数(mavlink_msg_xxx_decode)
            break;
        case 2:
            // id = 2 的消息对应的解码函数(mavlink_msg_xxx_decode)
            break;
        // ......
        default:
            break;
    }
}
