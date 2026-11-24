#include "button.h"

#include <avr/interrupt.h>

#include "board.h"
#include "task.h"

#define DEBOUNCE_MS    30
#define DEBOUNCE_TICKS (pdMS_TO_TICKS(DEBOUNCE_MS) + 1)

static button_handler_t handler;
static volatile uint8_t stable;  /* bit i = button i pressed */
static TickType_t last_change[BUTTON_COUNT];

/* Bit i set = button i pressed. The buttons pull their line low. */
static inline uint8_t read_raw(void)
{
    uint8_t raw = 0;

    if (!(BUTTON_BLUE_PIN & _BV(BUTTON_BLUE_BIT)))
        raw |= _BV(BUTTON_BLUE);
    if (!(BUTTON_YELLOW_PIN & _BV(BUTTON_YELLOW_BIT)))
        raw |= _BV(BUTTON_YELLOW);
    if (!(BUTTON_RED_PIN & _BV(BUTTON_RED_BIT)))
        raw |= _BV(BUTTON_RED);
    return raw;
}

/*
 * An edge is reported at once, then the button ignores bounces for
 * DEBOUNCE_TICKS. A change hidden by that window (very short tap) is
 * caught by the next scan from the tick hook.
 */
static void scan(BaseType_t *woken)
{
    TickType_t now = xTaskGetTickCountFromISR();
    uint8_t changed = read_raw() ^ stable;

    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        uint8_t bit = _BV(i);
        if (!(changed & bit) || (TickType_t)(now - last_change[i]) < DEBOUNCE_TICKS)
            continue;
        stable ^= bit;
        last_change[i] = now;
        if (handler)
            handler((button_id_t)i, stable & bit, woken);
    }
}

static void scan_from_pcint(void)
{
    BaseType_t woken = pdFALSE;
    scan(&woken);
    if (woken)
        taskYIELD();  /* the AVR port has no portYIELD_FROM_ISR */
}

ISR(PCINT2_vect)  /* blue (D5), red (D7) */
{
    scan_from_pcint();
}

ISR(PCINT0_vect)  /* yellow (D9) */
{
    scan_from_pcint();
}

void button_tick_from_isr(void)
{
    BaseType_t woken = pdFALSE;  /* the tick ISR switches context itself */
    scan(&woken);
}

void button_init(button_handler_t h)
{
    handler = h;
    /* Inputs. The modules have their own pull-up; the internal one is a
     * spare in case a cable comes loose (an open line reads "released"). */
    DDRD &= ~BUTTON_PORTD_MASK;
    PORTD |= BUTTON_PORTD_MASK;
    DDRB &= ~BUTTON_PORTB_MASK;
    PORTB |= BUTTON_PORTB_MASK;
    stable = read_raw();

    /* PCINT16..23 are PD0..7 and PCINT0..7 are PB0..7: the port masks
     * are also the pin change masks. */
    PCMSK2 |= BUTTON_PORTD_MASK;
    PCMSK0 |= BUTTON_PORTB_MASK;
    PCIFR = _BV(PCIF2) | _BV(PCIF0);
    PCICR |= _BV(PCIE2) | _BV(PCIE0);
}

bool button_is_pressed(button_id_t id)
{
    return stable & _BV(id);
}
