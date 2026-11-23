#include "button.h"

#include <avr/interrupt.h>

#include "board.h"
#include "task.h"

#define DEBOUNCE_MS    30
#define DEBOUNCE_TICKS (pdMS_TO_TICKS(DEBOUNCE_MS) + 1)

static button_handler_t handler;
static volatile uint8_t stable;  /* bit i = button i pressed */
static TickType_t last_change[BUTTON_COUNT];

static inline uint8_t read_raw(void)
{
    return (BUTTON_PIN & BUTTON_MASK) >> BUTTON_SHIFT;
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

ISR(PCINT2_vect)
{
    BaseType_t woken = pdFALSE;
    scan(&woken);
    if (woken)
        taskYIELD();  /* the AVR port has no portYIELD_FROM_ISR */
}

void button_tick_from_isr(void)
{
    BaseType_t woken = pdFALSE;  /* the tick ISR switches context itself */
    scan(&woken);
}

void button_init(button_handler_t h)
{
    handler = h;
    BUTTON_DDR &= ~BUTTON_MASK;
    BUTTON_PORT &= ~BUTTON_MASK;  /* Grove modules drive the line: no pull-up */
    stable = read_raw();

    PCMSK2 |= BUTTON_MASK;
    PCIFR = _BV(PCIF2);
    PCICR |= _BV(PCIE2);
}

bool button_is_pressed(button_id_t id)
{
    return stable & _BV(id);
}
