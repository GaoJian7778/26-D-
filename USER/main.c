/**
 * main.c - 地面站 2025电赛H题
 * STM32F407 FreeRTOS  SPI LCD  HC-04 BLE  7键
 */
#include "stm32f4xx.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "ble.h"
#include "Key.h"
#include "lcd.h"
#include "gui.h"
#include "ui.h"
#include "tasks.h"

int main(void)
{
    SystemInit();
    SystemCoreClockUpdate();    /* 必须: 否则外设时钟计算错误 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    LCD_Init();
    BLE_Init();
    for(volatile int d=0;d<1000000;d++); /* 等HC-04启动 */
    Key_Init();
    UI_Init();

    qBLEtoLCD = xQueueCreate(20, sizeof(MsgBLEData));

    xTaskCreate(Task_BLE_RX,  "BLE", 512, NULL, 2, &hTaskBLE);
    xTaskCreate(Task_KeyScan,"Key", 256, NULL, 3, &hTaskKey);
    xTaskCreate(Task_LCD,    "LCD",1024,NULL, 5, &hTaskLCD);

    vTaskStartScheduler();
    for(;;);
}
