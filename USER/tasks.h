/**
 * tasks.h - FreeRTOS 任务定义
 */

#ifndef __TASKS_H
#define __TASKS_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* ── 任务句柄 ── */
extern TaskHandle_t hTaskBLE;
extern TaskHandle_t hTaskKey;
extern TaskHandle_t hTaskLCD;

/* ── 队列句柄 ── */
extern QueueHandle_t qBLEtoLCD;
extern QueueHandle_t qKeytoBLE;

/* ── 队列消息 ── */
typedef struct {
    uint8_t type;  /* 0=动物 1=状态 2=结束 */
    uint8_t data[12];
} MsgBLEData;

/* ── 任务函数 ── */
void Task_BLE_RX(void *pvParameters);
void Task_KeyScan(void *pvParameters);
void Task_LCD(void    *pvParameters);

#endif
