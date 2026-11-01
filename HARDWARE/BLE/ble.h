/**
 * ble.h - HC-04 蓝牙 UART 驱动 (USART3, PC10=TX, PC11=RX, 9600bps)
 */

#ifndef __BLE_H
#define __BLE_H

#include "stm32f4xx.h"
#include "protocol.h"

/* ── 接收环形缓冲区 ── */
#define BLE_RX_BUF_SIZE   256

typedef struct {
    uint8_t  buffer[BLE_RX_BUF_SIZE];
    uint16_t head;
    uint16_t tail;
} RingBuf;

/* ── API ── */
void BLE_Init(void);
void BLE_Send(uint8_t *data, uint8_t len);
void BLE_SendFrame(Frame *f);

/* ── 接收处理 ── */
int  BLE_RecvByte(void);         /* 从缓冲区取一字节，-1=空 */
int  BLE_ParseFrame(Frame *f);   /* 尝试解析一帧，返回1=得到帧 */

/* ── UART发送完成标志（供任务使用） ── */
/* ── 连接状态 ── */
uint8_t BLE_IsConnected(void);

#endif
