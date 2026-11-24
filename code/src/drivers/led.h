#ifndef LED_H
#define LED_H

#include <stdbool.h>
#include <stdint.h>

#include "drivers/button.h"

/* LEDs of the Grove LED buttons, named after their button. */

void led_init(void);
void led_set(button_id_t id, bool on);

/* Red LED only, 0 (off) .. 255 (full): Timer0 PWM on OC0A, ~1 kHz.
 * led_set(BUTTON_RED, ...) goes back to plain on/off. */
void led_dim_red(uint8_t level);

#endif
