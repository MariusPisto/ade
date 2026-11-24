#include <string.h>

#include <avr/pgmspace.h>
#include <util/atomic.h>

#include "FreeRTOS.h"
#include "task.h"

#include "demo/demo.h"
#include "drivers/lcd.h"
#include "drivers/soft_i2c.h"
#include "drivers/ultrasonic.h"

/* Ultrasonic ranger (D2): distance, measurements per second and lost echoes. */

#define DEMO_PRIORITY   (tskIDLE_PRIORITY + 1)
#define DEMO_STACK_SIZE (configMINIMAL_STACK_SIZE + 64)
#define REFRESH_MS      250

static StaticTask_t demo_tcb;
static StackType_t demo_stack[DEMO_STACK_SIZE];

static volatile uint16_t last_mm = ULTRASONIC_NO_ECHO;
static volatile uint8_t measures;
static volatile uint8_t lost;

static void on_distance(uint16_t mm, BaseType_t *woken)
{
    (void)woken;
    last_mm = mm;
    measures++;
    if (mm == ULTRASONIC_NO_ECHO)
        lost++;
}

static void show_distance(uint16_t mm)
{
    char line[LCD_COLS];
    memset(line, ' ', sizeof(line));
    memcpy_P(line, PSTR("Dist"), 4);

    if (mm == ULTRASONIC_NO_ECHO) {
        memcpy_P(line + 6, PSTR("no echo"), 7);
    } else {
        demo_put_u16(line + 9, mm / 10);
        line[9] = '.';
        line[10] = '0' + mm % 10;
        memcpy_P(line + 12, PSTR("cm"), 2);
    }
    lcd_set_cursor(0, 0);
    lcd_write(line, sizeof(line));
}

static void show_rate(uint8_t per_second, uint8_t lost_per_second)
{
    char line[LCD_COLS];
    memset(line, ' ', sizeof(line));
    demo_put_u16(line + 3, per_second);
    memcpy_P(line + 3, PSTR("/s"), 2);
    memcpy_P(line + 7, PSTR("lost"), 4);
    demo_put_u16(line + 15, lost_per_second);
    lcd_set_cursor(0, 1);
    lcd_write(line, sizeof(line));
}

static void demo_task(void *arg)
{
    (void)arg;
    TickType_t wake = xTaskGetTickCount();
    uint8_t refreshes = 0;

    si2c_init();
    lcd_init();

    for (;;) {
        vTaskDelayUntil(&wake, pdMS_TO_TICKS(REFRESH_MS));

        uint16_t mm;
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
            mm = last_mm;
        }
        show_distance(mm);

        if (++refreshes == 1000 / REFRESH_MS) {
            uint8_t n, l;
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
                n = measures;
                l = lost;
                measures = 0;
                lost = 0;
            }
            show_rate(n, l);
            refreshes = 0;
        }
    }
}

void demo_init(void)
{
    ultrasonic_init(on_distance);
    xTaskCreateStatic(demo_task, "demo", DEMO_STACK_SIZE, NULL, DEMO_PRIORITY,
                      demo_stack, &demo_tcb);
}

void demo_tick_from_isr(void)
{
    ultrasonic_tick_from_isr();
}
