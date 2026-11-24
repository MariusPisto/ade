#ifndef BOARD_H
#define BOARD_H

#include <avr/io.h>

/* Pin map: Arduino Uno + Grove Base Shield v2 (see CONCEPTION.tex) */

/* Grove ultrasonic ranger on D2 (PD2, INT0): one pin for trigger and echo */
#define ULTRASONIC_PIN    PIND
#define ULTRASONIC_PORT   PORTD
#define ULTRASONIC_DDR    DDRD
#define ULTRASONIC_BIT    PD2

/* Grove light sensor v1.2 on A1 (ADC1) */
#define LIGHT_ADC_CHANNEL 1

/* Buttons 1..3 on D4, D5, D6 (PD4..PD6, PCINT20..22), active high */
#define BUTTON_PIN        PIND
#define BUTTON_PORT       PORTD
#define BUTTON_DDR        DDRD
#define BUTTON_SHIFT      PD4
#define BUTTON_MASK       (_BV(PD4) | _BV(PD5) | _BV(PD6))

/* On-board LED "L" (D13, PB5): lit on a fatal error. D13 goes to the
 * 4-digit display later, the error signal will move then. */
#define LED_FATAL_DDR     DDRB
#define LED_FATAL_PORT    PORTB
#define LED_FATAL_BIT     PB5

/* Software I2C bus (LCD, DIP switch): SCL = D7 (PD7), SDA = D8 (PB0) */
#define SI2C_SCL_DDR      DDRD
#define SI2C_SCL_PORT     PORTD
#define SI2C_SCL_PIN      PIND
#define SI2C_SCL_BIT      PD7

#define SI2C_SDA_DDR      DDRB
#define SI2C_SDA_PORT     PORTB
#define SI2C_SDA_PIN      PINB
#define SI2C_SDA_BIT      PB0

#endif
