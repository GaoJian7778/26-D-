/**
 * stm32f4xx_it.c - 中断服务函数
 *
 * 新增: USART3 空闲中断 (BLE接收)
 *       SysTick → FreeRTOS tick
 */

#include "stm32f4xx_it.h"
#include "stm32f4xx.h"
#include "ble.h"  /* 共享 emergency_flag */

/* ── SysTick ── */
extern void xPortSysTickHandler(void);
void SysTick_Handler(void)
{
    xPortSysTickHandler();
}

/* ── 硬错误 ── */
void NMI_Handler(void)           { ; }
void HardFault_Handler(void)     { while(1); }
void MemManage_Handler(void)     { while(1); }
void BusFault_Handler(void)      { while(1); }
void UsageFault_Handler(void)    { while(1); }
void DebugMon_Handler(void)      { ; }
/* SVC_Handler、PendSV_Handler 由 FreeRTOS port.c 提供 */

/* ── 急停键中断 (PE6, Key.c 中已实现) ── */
extern void EXTI9_5_IRQHandler(void);

/* ── USART1 BLE接收 → ble.c ── */
extern void USART1_IRQHandler(void);
