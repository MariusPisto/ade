#ifndef ULTRASONIC_H
#define ULTRASONIC_H

#include <stdint.h>

#include "FreeRTOS.h"

/*
 * Grove ultrasonic ranger: a ping every ULTRASONIC_PERIOD_MS from the tick
 * hook, echo timed by INT0 against Timer1 (the FreeRTOS tick), 4 us resolution.
 */

#define ULTRASONIC_PERIOD_MS 20
#define ULTRASONIC_NO_ECHO   0xFFFF

/* Runs in interrupt context: FromISR APIs only, forward `woken` to them.
 * mm is ULTRASONIC_NO_ECHO when nothing came back within the period. */
typedef void (*ultrasonic_handler_t)(uint16_t mm, BaseType_t *woken);

void ultrasonic_init(ultrasonic_handler_t handler);
void ultrasonic_tick_from_isr(void);  /* call from vApplicationTickHook */

#endif
