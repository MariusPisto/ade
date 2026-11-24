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

/*
 * Seeed Grove LED buttons, left to right: blue, yellow, red. Each one takes a
 * whole Grove port: yellow wire (Dn) = LED, active high (MOSFET on the
 * module); white wire (Dn+1) = button, active low (pull-up on the module).
 *   blue    port D4: LED D4 (PD4),        button D5 (PD5, PCINT21)
 *   red     port D6: LED D6 (PD6, OC0A),  button D7 (PD7, PCINT23)
 *   yellow  port D8: LED D8 (PB0),        button D9 (PB1, PCINT1)
 * The red LED sits on OC0A so Timer0 can dim it (hardware PWM).
 */
#define BUTTON_BLUE_PIN     PIND
#define BUTTON_BLUE_BIT     PD5
#define BUTTON_YELLOW_PIN   PINB
#define BUTTON_YELLOW_BIT   PB1
#define BUTTON_RED_PIN      PIND
#define BUTTON_RED_BIT      PD7
#define BUTTON_PORTD_MASK   (_BV(PD5) | _BV(PD7))  /* PCINT2 group */
#define BUTTON_PORTB_MASK   _BV(PB1)               /* PCINT0 group */

#define LED_BLUE_PORT       PORTD
#define LED_BLUE_DDR        DDRD
#define LED_BLUE_BIT        PD4
#define LED_YELLOW_PORT     PORTB
#define LED_YELLOW_DDR      DDRB
#define LED_YELLOW_BIT      PB0
#define LED_RED_PORT        PORTD
#define LED_RED_DDR         DDRD
#define LED_RED_BIT         PD6

/* On-board LED "L" (D13, PB5): lit on a fatal error. D13 goes to the
 * 4-digit display later, the error signal will move then. */
#define LED_FATAL_DDR     DDRB
#define LED_FATAL_PORT    PORTB
#define LED_FATAL_BIT     PB5

/* Software I2C bus (LCD, DIP switch) through the Grove I2C hub, wired with a
 * Grove -> male jumper cable: SCL = D10 (PB2), SDA = D11 (PB3) */
#define SI2C_SCL_DDR      DDRB
#define SI2C_SCL_PORT     PORTB
#define SI2C_SCL_PIN      PINB
#define SI2C_SCL_BIT      PB2

#define SI2C_SDA_DDR      DDRB
#define SI2C_SDA_PORT     PORTB
#define SI2C_SDA_PIN      PINB
#define SI2C_SDA_BIT      PB3

#endif
