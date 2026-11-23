#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include <avr/io.h>

/* Course ATMega328 port: 1 ms tick from Timer1 (compare match A). */
#define configCPU_CLOCK_HZ                      F_CPU
#define configTICK_RATE_HZ                      ((TickType_t)1000)
#define configUSE_16_BIT_TICKS                  1
#define portPOINTER_SIZE_TYPE                   uint16_t  /* missing from the course port */

#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configIDLE_SHOULD_YIELD                 1
#define configMAX_PRIORITIES                    4
#define configMAX_TASK_NAME_LEN                 8
/* No separate interrupt stack on AVR: every task stack must also hold the
 * deepest ISR, i.e. the tick (34 B context) whose hook posts a button event
 * (~70 B). 140 B = idle task + that reserve + the 16 B overflow guard.
 * Other tasks: their own needs + configMINIMAL_STACK_SIZE. */
#define configMINIMAL_STACK_SIZE                140

#define configSUPPORT_STATIC_ALLOCATION         1
#define configSUPPORT_DYNAMIC_ALLOCATION        0
#define configKERNEL_PROVIDED_STATIC_MEMORY     1

#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   1
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           0
#define configUSE_QUEUE_SETS                    0
#define configQUEUE_REGISTRY_SIZE               0
#define configUSE_TIMERS                        0
#define configTIMER_TASK_STACK_DEPTH            configMINIMAL_STACK_SIZE  /* required by V11.1, unused */
#define configUSE_CO_ROUTINES                   0

#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     1
#define configCHECK_FOR_STACK_OVERFLOW          2
#define configUSE_MALLOC_FAILED_HOOK            0

#define configUSE_TRACE_FACILITY                0
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_STATS_FORMATTING_FUNCTIONS    0
#define configENABLE_BACKWARD_COMPATIBILITY     0

void app_fatal(void);
#define configASSERT(x)                         do { if (!(x)) app_fatal(); } while (0)

#define INCLUDE_vTaskDelay                      1
#define INCLUDE_xTaskDelayUntil                 1
#define INCLUDE_vTaskSuspend                    1
#define INCLUDE_uxTaskGetStackHighWaterMark     1
#define INCLUDE_vTaskPrioritySet                0
#define INCLUDE_uxTaskPriorityGet               0
#define INCLUDE_vTaskDelete                     0

#endif
