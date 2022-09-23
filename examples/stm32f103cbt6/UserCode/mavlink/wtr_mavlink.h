/**
 * @file wtr_mavlink.h
 * @author X. Y.
 * @brief 移植到 stm32 的 mavlink（只需要包含这个头文件就够了）
 * @version 0.5
 * @date 2022-09-23
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"
#include "mavlink_types.h"
#include "usart.h"

/***********************************
 * 以下为移植到新平台后建议的配置
 ***********************************/

// 系统 ID 和 组件 ID
static mavlink_system_t mavlink_system = {
    1, // System ID (1-255)
    1  // Component ID (1-255)
};

// MavLink 能使用的最大通道数
#define MAVLINK_COMM_NUM_BUFFERS 4

/***********************************
 * 以上为移植到新平台后建议的配置
 ***********************************/

// mavlink 的句柄
typedef struct
{
    UART_HandleTypeDef *huart; // 该通道对应的串口句柄
    uint8_t rx_buffer;         // 接收缓冲
    mavlink_message_t msg;     // the message data
    mavlink_status_t status;   // 统计信息
} wtrMavlink_handle_t;

// mavlink 支持多个通道，使用全局句柄方便对每一个通道配置（通常，一个通道对应一个串口）
extern wtrMavlink_handle_t hMAVLink[MAVLINK_COMM_NUM_BUFFERS];

/*************************
 * 以下为针对 mavlink 的配置
 *************************/

// 开启便利函数
#define MAVLINK_USE_CONVENIENCE_FUNCTIONS

#ifdef MAVLINK_USE_CONVENIENCE_FUNCTIONS
static inline void wtrMavlink_SEND_UART_BYTES(mavlink_channel_t chan, const uint8_t *buf, uint16_t len)
{
    if (chan < MAVLINK_COMM_NUM_BUFFERS) {
        while (HAL_UART_Transmit(hMAVLink[chan].huart, (uint8_t *)buf, len, HAL_MAX_DELAY) == HAL_BUSY)
            ;
    }
}
#define MAVLINK_SEND_UART_BYTES(chan, buf, len) wtrMavlink_SEND_UART_BYTES(chan, buf, len)
#endif // MAVLINK_USE_CONVENIENCE_FUNCTIONS

#define MAVLINK_GET_CHANNEL_STATUS
static inline mavlink_status_t *mavlink_get_channel_status(uint8_t chan)
{
    return &hMAVLink[chan].status;
}

#define MAVLINK_GET_CHANNEL_BUFFER
static inline mavlink_message_t *mavlink_get_channel_buffer(uint8_t chan)
{
    return &hMAVLink[chan].msg;
}

#include "mavlink.h"

/***************************
 * 以下为针对 wtrMavlink 函数
 ***************************/

/**
 * @brief 将串口和 mavlink 的通道绑定
 *
 * @param huart 串口句柄
 * @param chan 通道号
 */
void wtrMavlink_BindChannel(UART_HandleTypeDef *huart, mavlink_channel_t chan);

/**
 * @brief 开启接收中断（记得先绑定通道和串口）
 *
 * @param chan 通道号
 * @return 0：成功；-1：未绑定串口；HAL_BUSY：串口忙；HAL_ERROR：中断开启出错
 */
int wtrMavlink_StartReceiveIT(mavlink_channel_t chan);

/**
 * @brief 接收到完整消息且校验通过后会调用这个函数。在这个函数里调用解码函数就可以向结构体写入收到的数据
 *
 * @param msg 接收到的消息
 * @return
 */
void wtrMavlink_MsgRxCpltCallback(mavlink_message_t *msg);

/**
 * @brief 这个函数需要在 HAL_UART_RxCpltCallback 中调用
 *
 * @param huart 收到数据的串口（函数内部有判断，只有这个 huart 是 channel 对应的 huart 时，该函数才会处理）
 * @param chan
 */
static inline void wtrMavlink_UARTRxCpltCallback(UART_HandleTypeDef *huart, mavlink_channel_t chan)
{
    if (huart == hMAVLink[chan].huart) {
        if (mavlink_parse_char(chan, hMAVLink[chan].rx_buffer, &(hMAVLink[chan].msg), &(hMAVLink[chan].status))) {
            wtrMavlink_MsgRxCpltCallback(&(hMAVLink[chan].msg));
        }
        HAL_UART_Receive_IT(huart, &(hMAVLink[chan].rx_buffer), 1);
    }
}

#ifdef __cplusplus
}
#endif
