#ifndef LIGHT_H
#define LIGHT_H

#include <stdint.h>

/* Grove light sensor v1.2: 0 (dark) .. 1023 (bright).
 * Same owner as the ADC; call light_sample() periodically. */

void light_init(void);
uint16_t light_sample(void);  /* one conversion, updates the level; returns the raw value */
uint16_t light_level(void);   /* smoothed (moving average, ~4 samples) */

#endif
