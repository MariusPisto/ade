#include "soft_i2c.h"

#include <util/delay.h>

#include "board.h"

#define HALF_PERIOD_US     10
#define STRETCH_TIMEOUT_US 1000

/*
 * Open-drain emulation: a line is either driven low or released (input with
 * pull-up). Single-bit accesses compile to sbi/cbi, so they are atomic against
 * ISRs that touch other pins of the same port.
 */

static inline void scl_low(void)
{
    SI2C_SCL_PORT &= ~_BV(SI2C_SCL_BIT);
    SI2C_SCL_DDR |= _BV(SI2C_SCL_BIT);
}

static inline void sda_low(void)
{
    SI2C_SDA_PORT &= ~_BV(SI2C_SDA_BIT);
    SI2C_SDA_DDR |= _BV(SI2C_SDA_BIT);
}

static inline void sda_release(void)
{
    SI2C_SDA_DDR &= ~_BV(SI2C_SDA_BIT);
    SI2C_SDA_PORT |= _BV(SI2C_SDA_BIT);
}

static inline bool sda_read(void)
{
    return SI2C_SDA_PIN & _BV(SI2C_SDA_BIT);
}

/* Releases SCL and waits while a slave stretches the clock. */
static void scl_release(void)
{
    SI2C_SCL_DDR &= ~_BV(SI2C_SCL_BIT);
    SI2C_SCL_PORT |= _BV(SI2C_SCL_BIT);
    for (uint16_t t = 0; t < STRETCH_TIMEOUT_US; t++) {
        if (SI2C_SCL_PIN & _BV(SI2C_SCL_BIT))
            return;
        _delay_us(1);
    }
}

static void write_bit(bool bit)
{
    if (bit)
        sda_release();
    else
        sda_low();
    _delay_us(HALF_PERIOD_US);
    scl_release();
    _delay_us(HALF_PERIOD_US);
    scl_low();
}

static bool read_bit(void)
{
    sda_release();
    _delay_us(HALF_PERIOD_US);
    scl_release();
    _delay_us(HALF_PERIOD_US);
    bool bit = sda_read();
    scl_low();
    return bit;
}

void si2c_init(void)
{
    sda_release();
    scl_release();

    /* A slave reset mid-transfer may hold SDA low: clock it out. */
    for (uint8_t i = 0; i < 9 && !sda_read(); i++) {
        scl_low();
        _delay_us(HALF_PERIOD_US);
        scl_release();
        _delay_us(HALF_PERIOD_US);
    }
    si2c_stop();
}

bool si2c_start(uint8_t addr, bool read)
{
    sda_release();
    scl_release();
    _delay_us(HALF_PERIOD_US);
    sda_low();
    _delay_us(HALF_PERIOD_US);
    scl_low();
    return si2c_write((uint8_t)(addr << 1) | read);
}

bool si2c_write(uint8_t byte)
{
    for (uint8_t mask = 0x80; mask; mask >>= 1)
        write_bit(byte & mask);
    return !read_bit();
}

uint8_t si2c_read(bool ack)
{
    uint8_t byte = 0;
    for (uint8_t i = 0; i < 8; i++)
        byte = (uint8_t)(byte << 1) | read_bit();
    write_bit(!ack);
    return byte;
}

void si2c_stop(void)
{
    sda_low();
    _delay_us(HALF_PERIOD_US);
    scl_release();
    _delay_us(HALF_PERIOD_US);
    sda_release();
    _delay_us(HALF_PERIOD_US);
}

bool si2c_probe(uint8_t addr)
{
    bool ack = si2c_start(addr, false);
    si2c_stop();
    return ack;
}
