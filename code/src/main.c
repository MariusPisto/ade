#include <stdbool.h>
#include <stdint.h>

#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "board.h"
#include "drivers/button.h"
#include "drivers/lcd.h"
#include "drivers/soft_i2c.h"

/* Driver demo: each button press is counted and shown on the LCD. */

#define DEMO_PRIORITY   (tskIDLE_PRIORITY + 1)
#define DEMO_STACK_SIZE (configMINIMAL_STACK_SIZE + 64)
#define EVENT_QUEUE_LEN 8

typedef struct {
    uint8_t id;
    bool pressed;
} button_event_t;

static StaticQueue_t event_queue_buf;
static uint8_t event_queue_storage[EVENT_QUEUE_LEN * sizeof(button_event_t)];
static QueueHandle_t event_queue;

static StaticTask_t demo_tcb;
static StackType_t demo_stack[DEMO_STACK_SIZE];

static void on_button(button_id_t id, bool pressed, BaseType_t *woken)
{
    button_event_t ev = { .id = id, .pressed = pressed };
    xQueueSendFromISR(event_queue, &ev, woken);
}

static void show_counts(const uint8_t counts[BUTTON_COUNT])
{
    char line[LCD_COLS];
    uint8_t n = 0;

    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        line[n++] = '1' + i;
        line[n++] = ':';
        line[n++] = '0' + counts[i] / 10 % 10;
        line[n++] = '0' + counts[i] % 10;
        line[n++] = ' ';
    }
    lcd_set_cursor(0, 1);
    lcd_write(line, n - 1);
}

static void demo_task(void *arg)
{
    (void)arg;
    uint8_t counts[BUTTON_COUNT] = { 0 };
    button_event_t ev;

    si2c_init();
    lcd_init();
    lcd_print_P(PSTR("ADE ready"));
    show_counts(counts);

    for (;;) {
        xQueueReceive(event_queue, &ev, portMAX_DELAY);

        char digit = '1' + ev.id;
        lcd_set_cursor(0, 0);
        lcd_print_P(PSTR("Button "));
        lcd_write(&digit, 1);
        lcd_print_P(ev.pressed ? PSTR(" down ") : PSTR(" up   "));

        if (ev.pressed) {
            counts[ev.id]++;
            show_counts(counts);
        }
    }
}

int main(void)
{
    event_queue = xQueueCreateStatic(EVENT_QUEUE_LEN, sizeof(button_event_t),
                                     event_queue_storage, &event_queue_buf);
    button_init(on_button);

    xTaskCreateStatic(demo_task, "demo", DEMO_STACK_SIZE, NULL, DEMO_PRIORITY,
                      demo_stack, &demo_tcb);
    vTaskStartScheduler();

    for (;;) {
    }
}

void vApplicationTickHook(void)
{
    button_tick_from_isr();
}

void app_fatal(void)
{
    cli();
    LED_FATAL_DDR |= _BV(LED_FATAL_BIT);
    LED_FATAL_PORT |= _BV(LED_FATAL_BIT);
    for (;;) {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task;
    (void)name;
    app_fatal();
}
