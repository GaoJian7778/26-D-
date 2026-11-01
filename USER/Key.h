/**
 * Key.h - 7键驱动
 * KEY1(↑)=PD8 KEY2(↓)=PD10 KEY3(←)=PD12 KEY4(→)=PD14
 * KEY5(确认)=PC6 KEY6(返回)=PC8 KEY7(急停)=PA8
 */
#ifndef __KEY_H
#define __KEY_H
#include "stm32f4xx.h"

#define KEY_UP         0
#define KEY_DOWN       1
#define KEY_LEFT       2
#define KEY_RIGHT      3
#define KEY_CONFIRM    4
#define KEY_BACK       5
#define KEY_EMERGENCY  6

#define KEY_EVENT_DOWN   0x01
#define KEY_EVENT_UP     0x02
#define KEY_EVENT_LONG   0x03
#define KEY_EVENT_REPEAT 0x04

typedef struct { uint8_t key, event; } KeyEvent;

void Key_Init(void);
void Key_Scan(void);
int  Key_GetEvent(KeyEvent *evt);
extern volatile uint8_t emergency_flag;
#endif
