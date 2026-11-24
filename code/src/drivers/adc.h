#ifndef ADC_H
#define ADC_H

#include <stdint.h>

/* 10-bit ADC, AVCC (5 V) reference. Blocking read (~104 us).
 * Not thread-safe: a single task owns the ADC. */

void adc_init(void);
uint16_t adc_read(uint8_t channel);

#endif
