#ifndef BUTTON_H
#define BUTTON_H

#include <stdbool.h>

#include "FreeRTOS.h"

typedef enum {
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_COUNT
} button_id_t;

/* Runs in interrupt context: FromISR APIs only, forward `woken` to them. */
typedef void (*button_handler_t)(button_id_t id, bool pressed, BaseType_t *woken);

void button_init(button_handler_t handler);
bool button_is_pressed(button_id_t id);  /* debounced state */
void button_tick_from_isr(void);         /* call from vApplicationTickHook */

#endif
