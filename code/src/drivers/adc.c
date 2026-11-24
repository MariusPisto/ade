#include "adc.h"

#include <avr/io.h>

void adc_init(void)
{
    ADMUX = _BV(REFS0);
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);  /* 16 MHz / 128 */
}

uint16_t adc_read(uint8_t channel)
{
    ADMUX = _BV(REFS0) | (channel & 0x07);
    ADCSRA |= _BV(ADSC);
    while (ADCSRA & _BV(ADSC)) {
    }
    return ADC;
}
