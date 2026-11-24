#include "led.h"

#include <avr/io.h>
#include <util/atomic.h>

#include "board.h"

void led_init(void)
{
    LED_BLUE_PORT &= ~_BV(LED_BLUE_BIT);
    LED_BLUE_DDR |= _BV(LED_BLUE_BIT);
    LED_YELLOW_PORT &= ~_BV(LED_YELLOW_BIT);
    LED_YELLOW_DDR |= _BV(LED_YELLOW_BIT);
    LED_RED_PORT &= ~_BV(LED_RED_BIT);
    LED_RED_DDR |= _BV(LED_RED_BIT);

    /* Timer0: fast PWM, clk/64 -> 976 Hz. OC0A stays disconnected (the pin
     * follows PORTD) until led_dim_red() asks for a level. */
    TCCR0A = _BV(WGM01) | _BV(WGM00);
    TCCR0B = _BV(CS01) | _BV(CS00);
}

/* TCCR0A is outside the sbi/cbi range: guard its read-modify-write,
 * the safety task may preempt the HMI task here. */
static void red_pwm_off(void)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        TCCR0A &= ~_BV(COM0A1);
    }
}

void led_set(button_id_t id, bool on)
{
    switch (id) {
    case BUTTON_BLUE:
        if (on)
            LED_BLUE_PORT |= _BV(LED_BLUE_BIT);
        else
            LED_BLUE_PORT &= ~_BV(LED_BLUE_BIT);
        break;
    case BUTTON_YELLOW:
        if (on)
            LED_YELLOW_PORT |= _BV(LED_YELLOW_BIT);
        else
            LED_YELLOW_PORT &= ~_BV(LED_YELLOW_BIT);
        break;
    case BUTTON_RED:
        red_pwm_off();
        if (on)
            LED_RED_PORT |= _BV(LED_RED_BIT);
        else
            LED_RED_PORT &= ~_BV(LED_RED_BIT);
        break;
    default:
        break;
    }
}

void led_dim_red(uint8_t level)
{
    if (level == 0) {
        led_set(BUTTON_RED, false);
        return;
    }
    OCR0A = level;
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
        TCCR0A |= _BV(COM0A1);  /* non-inverting: high for level/256 */
    }
}
