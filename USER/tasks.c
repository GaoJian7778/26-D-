/**
 * tasks.c - D题 FreeRTOS 任务
 */
#include "tasks.h"
#include "ble.h"
#include "Key.h"
#include "ui.h"
#include "gui.h"
#include <string.h>

QueueHandle_t qBLEtoLCD=NULL;
TaskHandle_t hTaskBLE=NULL,hTaskKey=NULL,hTaskLCD=NULL;

void Task_BLE_RX(void *p){
    (void)p;Frame f;
    for(;;){
        if(emergency_flag){Frame e;proto_build_emergency(&e);BLE_SendFrame(&e);emergency_flag=0;GUI_ShowEmergency();}
        if(BLE_ParseFrame(&f)&&f.dir==DIR_FROM_DRONE){
            MsgBLEData m;
            switch(f.cmd){
            case CMD_ANIMAL: m.type=0;memcpy(m.data,f.data,4);xQueueSend(qBLEtoLCD,&m,0);break;
            case CMD_STATUS: m.type=1;memcpy(m.data,f.data,4);xQueueSend(qBLEtoLCD,&m,0);break;
            case CMD_MISSION_DONE: m.type=2;memcpy(m.data,f.data,12);xQueueSend(qBLEtoLCD,&m,0);break;
            }
        }
        vTaskDelay(10);
    }
}

void Task_KeyScan(void *p){
    (void)p;KeyEvent e;
    for(;;){Key_Scan();while(Key_GetEvent(&e))UI_ProcessKey(&e);vTaskDelay(10);}
}

void Task_LCD(void *p){
    (void)p;MsgBLEData m;
    for(;;){
        while(xQueueReceive(qBLEtoLCD,&m,0)==pdTRUE){
            switch(m.type){
            case 0: UI_OnTarget(m.data[0],m.data[1],m.data[2],m.data[3]);break;
            case 1:
                    if(m.data[3]==STATUS_QR_SCAN){UI_OnQR(m.data[0],m.data[1]);break;} /* QR不更新位置 */
                    UI_OnStatus(m.data[0],m.data[1],m.data[2],m.data[3]);break;
            case 2:{uint16_t c[3];
                    for(int i=0;i<3;i++)c[i]=((uint16_t)m.data[i*2]<<8)|m.data[i*2+1];
                    uint16_t el=((uint16_t)m.data[10]<<8)|m.data[11];
                    UI_OnMissionDone(c,el);}break;
            }
        }
        UI_Update();
        vTaskDelay(15);
    }
}
