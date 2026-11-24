#include "ultrasonic.h"

#include <stdbool.h>

#include <avr/interrupt.h>
#include <util/delay.h>

#include "board.h"
#include "task.h"

#define PERIOD_TICKS       pdMS_TO_TICKS(ULTRASONIC_PERIOD_MS)
#define MAX_MM             3500

/* The port runs Timer1 in CTC with a /64 prescaler: one count every 4 us. */
#define COUNTS_PER_TICK    ((uint16_t)(configCPU_CLOCK_HZ / 64 / configTICK_RATE_HZ))
#define MM_PER_COUNT_Q10   703  /* 4 us * 343 m/s / 2 = 0.686 mm, times 1024 */

typedef enum { IDLE, WAIT_RISE, WAIT_FALL } state_t;

/* Only touched by the tick hook and INT0, which never nest. */
static ultrasonic_handler_t handler;
static state_t state;
static TickType_t ticks_since_ping;
static TickType_t start_tick;
static uint16_t start_count;

static inline bool echo_high(void)
{
    return ULTRASONIC_PIN & _BV(ULTRASONIC_BIT);
}

static inline void echo_irq(bool on)
{
    if (on) {
        EIFR = _BV(INTF0);
        EIMSK |= _BV(INT0);
    } else {
        EIMSK &= ~_BV(INT0);
    }
}

/* A compare match not yet serviced means TCNT1 already wrapped into the next tick. */
static void timestamp(TickType_t *tick, uint16_t *count)
{
    uint16_t c = TCNT1;
    TickType_t t = xTaskGetTickCountFromISR();

    if ((TIFR1 & _BV(OCF1A)) && c < COUNTS_PER_TICK / 2)
        t++;
    *tick = t;
    *count = c;
}

static void ping(void)
{
    uint8_t bit = _BV(ULTRASONIC_BIT);

    ULTRASONIC_PORT &= ~bit;
    ULTRASONIC_DDR |= bit;
    _delay_us(2);
    ULTRASONIC_PORT |= bit;
    _delay_us(10);
    ULTRASONIC_PORT &= ~bit;
    ULTRASONIC_DDR &= ~bit;

    state = WAIT_RISE;
    echo_irq(true);
}

static uint16_t to_mm(uint32_t counts)
{
    uint32_t mm = (counts * MM_PER_COUNT_Q10) >> 10;
    return mm > MAX_MM ? ULTRASONIC_NO_ECHO : (uint16_t)mm;
}

ISR(INT0_vect)
{
    TickType_t tick;
    uint16_t count;
    timestamp(&tick, &count);

    if (echo_high()) {
        if (state == WAIT_RISE) {
            start_tick = tick;
            start_count = count;
            state = WAIT_FALL;
        }
        return;
    }
    if (state != WAIT_FALL)
        return;

    echo_irq(false);
    state = IDLE;

    uint32_t counts = (uint32_t)(TickType_t)(tick - start_tick) * COUNTS_PER_TICK
                      + count - start_count;
    BaseType_t woken = pdFALSE;
    if (handler)
        handler(to_mm(counts), &woken);
    if (woken)
        taskYIELD();  /* the AVR port has no portYIELD_FROM_ISR */
}

void ultrasonic_tick_from_isr(void)
{
    if (++ticks_since_ping < PERIOD_TICKS)
        return;
    ticks_since_ping = PERIOD_TICKS;  /* hold while the line is still busy */

    if (state != IDLE) {
        BaseType_t woken = pdFALSE;  /* the tick ISR switches context itself */
        echo_irq(false);
        state = IDLE;
        if (handler)
            handler(ULTRASONIC_NO_ECHO, &woken);
    }

    /* The sensor cannot ping again before its previous echo is over. */
    if (!echo_high()) {
        ping();
        ticks_since_ping = 0;
    }
}

void ultrasonic_init(ultrasonic_handler_t h)
{
    handler = h;
    state = IDLE;
    ticks_since_ping = PERIOD_TICKS;

    ULTRASONIC_DDR &= ~_BV(ULTRASONIC_BIT);
    ULTRASONIC_PORT &= ~_BV(ULTRASONIC_BIT);
    EICRA = (EICRA & ~(_BV(ISC01) | _BV(ISC00))) | _BV(ISC00);  /* any edge */
}
