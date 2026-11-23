#include "lcd.h"

#include <avr/pgmspace.h>
#include <util/delay.h>

#include "soft_i2c.h"

#define LCD_ADDR       0x3E
#define CTRL_COMMAND   0x80
#define CTRL_DATA      0x40

#define CMD_CLEAR      0x01
#define CMD_ENTRY_MODE 0x06  /* left to right, no shift */
#define CMD_DISPLAY    0x08
#define DISPLAY_ON     0x04
#define CMD_FUNCTION   0x28  /* 2 lines, 5x8 font */
#define CMD_CGRAM      0x40
#define CMD_DDRAM      0x80

static bool command(uint8_t cmd)
{
    bool ok = si2c_start(LCD_ADDR, false) && si2c_write(CTRL_COMMAND) && si2c_write(cmd);
    si2c_stop();
    return ok;
}

static bool data_begin(void)
{
    return si2c_start(LCD_ADDR, false) && si2c_write(CTRL_DATA);
}

bool lcd_init(void)
{
    _delay_ms(50);  /* power-up */

    if (!command(CMD_FUNCTION))
        return false;
    _delay_us(4500);
    command(CMD_FUNCTION);
    _delay_us(150);
    command(CMD_FUNCTION);

    lcd_display(true);
    lcd_clear();
    return command(CMD_ENTRY_MODE);
}

void lcd_clear(void)
{
    command(CMD_CLEAR);
    _delay_ms(2);
}

void lcd_display(bool on)
{
    command(CMD_DISPLAY | (on ? DISPLAY_ON : 0));
}

void lcd_set_cursor(uint8_t col, uint8_t row)
{
    command(CMD_DDRAM | (row ? 0x40 : 0x00) | col);
}

void lcd_write(const char *buf, uint8_t len)
{
    if (data_begin())
        while (len--)
            si2c_write((uint8_t)*buf++);
    si2c_stop();
}

void lcd_print(const char *str)
{
    if (data_begin())
        while (*str)
            si2c_write((uint8_t)*str++);
    si2c_stop();
}

void lcd_print_P(const char *str)
{
    char c;
    if (data_begin())
        while ((c = pgm_read_byte(str++)))
            si2c_write((uint8_t)c);
    si2c_stop();
}

void lcd_create_char(uint8_t slot, const uint8_t rows[8])
{
    command(CMD_CGRAM | (uint8_t)((slot & 0x07) << 3));
    lcd_write((const char *)rows, 8);
}
