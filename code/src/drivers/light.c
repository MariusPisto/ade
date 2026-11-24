#include "light.h"

#include <avr/io.h>

#include "adc.h"
#include "board.h"

#define SMOOTH_SHIFT 2  /* exponential moving average, weight 1/4 */

static uint16_t acc;    /* level << SMOOTH_SHIFT */

void light_init(void)
{
    adc_init();
    DIDR0 |= _BV(LIGHT_ADC_CHANNEL);  /* analog only: no digital input buffer */
    acc = adc_read(LIGHT_ADC_CHANNEL) << SMOOTH_SHIFT;
}

uint16_t light_sample(void)
{
    uint16_t raw = adc_read(LIGHT_ADC_CHANNEL);
    acc = acc - (acc >> SMOOTH_SHIFT) + raw;
    return raw;
}

uint16_t light_level(void)
{
    return acc >> SMOOTH_SHIFT;
}
