/**
 * FreeRTOSConfig.h - 针对 STM32F407VGT6 (168MHz)
 * 基于真省赛参考工程，调整主频为168MHz
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f4xx.h"

#define configUSE_PREEMPTION             1
#define configUSE_IDLE_HOOK              0
#define configUSE_TICK_HOOK              0
#define configCPU_CLOCK_HZ               ( ( unsigned long ) 168000000 )
#define configTICK_RATE_HZ               ( ( TickType_t ) 1000 )
#define configMAX_PRIORITIES             ( 5 )
#define configMINIMAL_STACK_SIZE         ( ( unsigned short ) 128 )
#define configTOTAL_HEAP_SIZE            ( ( size_t ) ( 20 * 1024 ) )
#define configMAX_TASK_NAME_LEN          ( 16 )
#define configUSE_TRACE_FACILITY         1
#define configUSE_16_BIT_TICKS           0
#define configIDLE_SHOULD_YIELD          1

#define configUSE_MUTEXES                1
#define configUSE_COUNTING_SEMAPHORES    1

#define INCLUDE_vTaskPrioritySet         1
#define INCLUDE_uxTaskPriorityGet        1
#define INCLUDE_vTaskDelete              1
#define INCLUDE_vTaskSuspend             1
#define INCLUDE_vTaskDelayUntil          1
#define INCLUDE_vTaskDelay               1

/* ── Cortex-M4 中断映射 ── */
#define xPortPendSVHandler    PendSV_Handler
#define vPortSVCHandler       SVC_Handler

/* ── RTOS 内核中断优先级 ── */
#define configKERNEL_INTERRUPT_PRIORITY          (15 << 4)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY     (5 << 4)
#define configMAX_API_CALL_INTERRUPT_PRIORITY    configMAX_SYSCALL_INTERRUPT_PRIORITY

#endif /* FREERTOS_CONFIG_H */
