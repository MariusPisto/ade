#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

#include "FreeRTOS.h"

/* The 3 Grove LED buttons, left to right on the bomb. */
typedef enum {
    BUTTON_BLUE,
    BUTTON_YELLOW,
    BUTTON_RED,
    BUTTON_COUNT
} button_id_t;

/* Runs in interrupt context: FromISR APIs only, forward `woken` to them. */
typedef void (*button_handler_t)(button_id_t id, bool pressed, BaseType_t *woken);

void button_init(button_handler_t handler);
bool button_is_pressed(button_id_t id);  /* debounced state */
void button_tick_from_isr(void);         /* call from vApplicationTickHook */

#endif
