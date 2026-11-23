#ifndef LCD_H
#define LCD_H

#include <stdbool.h>
#include <stdint.h>

/* Grove 16x2 LCD (JHD1802, I2C 0x3E) on the software I2C bus. */

#define LCD_COLS 16
#define LCD_ROWS 2

bool lcd_init(void);  /* si2c_init() first; false if the LCD does not answer */
void lcd_clear(void);
void lcd_display(bool on);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_write(const char *buf, uint8_t len);
void lcd_print(const char *str);
void lcd_print_P(const char *str);  /* string in flash (PSTR) */

/* Custom glyph in CGRAM slot 0..7, shown by writing the slot number.
 * Prefer slots 1..7 with lcd_print (0 ends a string). Call lcd_set_cursor after. */
void lcd_create_char(uint8_t slot, const uint8_t rows[8]);

#endif
