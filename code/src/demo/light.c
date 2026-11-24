#include <string.h>

#include <avr/pgmspace.h>

#include "FreeRTOS.h"
#include "task.h"

#include "demo/demo.h"
#include "drivers/lcd.h"
#include "drivers/light.h"
#include "drivers/soft_i2c.h"

/* Light sensor (A1): raw and smoothed values, plus a bar graph of the level. */

#define DEMO_PRIORITY    (tskIDLE_PRIORITY + 1)
#define DEMO_STACK_SIZE  (configMINIMAL_STACK_SIZE + 64)
#define SAMPLE_MS        20
#define SAMPLES_PER_SHOW 10
#define LCD_FULL_BLOCK   0xFF

static StaticTask_t demo_tcb;
static StackType_t demo_stack[DEMO_STACK_SIZE];

static void show(uint16_t raw, uint16_t level)
{
    char line[LCD_COLS];

    memset(line, ' ', sizeof(line));
    memcpy_P(line, PSTR("Raw"), 3);
    demo_put_u16(line + 8, raw);
    memcpy_P(line + 9, PSTR("Avg"), 3);
    demo_put_u16(line + 16, level);
    lcd_set_cursor(0, 0);
    lcd_write(line, sizeof(line));

    uint8_t blocks = (uint8_t)(((uint32_t)level * LCD_COLS + 512) / 1024);
    memset(line, ' ', sizeof(line));
    memset(line, LCD_FULL_BLOCK, blocks);
    lcd_set_cursor(0, 1);
    lcd_write(line, sizeof(line));
}

static void demo_task(void *arg)
{
    (void)arg;
    TickType_t wake = xTaskGetTickCount();
    uint8_t samples = 0;

    si2c_init();
    lcd_init();
    light_init();

    for (;;) {
        vTaskDelayUntil(&wake, pdMS_TO_TICKS(SAMPLE_MS));
        uint16_t raw = light_sample();

        if (++samples == SAMPLES_PER_SHOW) {
            samples = 0;
            show(raw, light_level());
        }
    }
}

void demo_init(void)
{
    xTaskCreateStatic(demo_task, "demo", DEMO_STACK_SIZE, NULL, DEMO_PRIORITY,
                      demo_stack, &demo_tcb);
}

void demo_tick_from_isr(void)
{
}
