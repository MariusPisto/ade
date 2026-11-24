#include <stdbool.h>

#include <avr/pgmspace.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "demo/demo.h"
#include "drivers/button.h"
#include "drivers/lcd.h"
#include "drivers/led.h"
#include "drivers/soft_i2c.h"

/*
 * LED buttons (blue D4/D5, red D6/D7, yellow D8/D9): last event on line 1,
 * press count of each button on line 2. A button lights while held; at rest
 * the red one glows faintly (Timer0 PWM).
 */

#define RED_IDLE_LEVEL  12

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

static const char names[BUTTON_COUNT] = { 'B', 'Y', 'R' };

static void show_led(button_id_t id, bool pressed)
{
    if (id == BUTTON_RED && !pressed)
        led_dim_red(RED_IDLE_LEVEL);
    else
        led_set(id, pressed);
}

static void show_counts(const uint8_t counts[BUTTON_COUNT])
{
    char line[LCD_COLS];
    uint8_t n = 0;

    for (uint8_t i = 0; i < BUTTON_COUNT; i++) {
        line[n++] = names[i];
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
    led_init();
    show_led(BUTTON_RED, false);
    lcd_print_P(PSTR("ADE ready"));
    show_counts(counts);

    for (;;) {
        xQueueReceive(event_queue, &ev, portMAX_DELAY);

        show_led(ev.id, ev.pressed);

        lcd_set_cursor(0, 0);
        lcd_print_P(PSTR("Button "));
        lcd_write(&names[ev.id], 1);
        lcd_print_P(ev.pressed ? PSTR(" down ") : PSTR(" up   "));

        if (ev.pressed) {
            counts[ev.id]++;
            show_counts(counts);
        }
    }
}

void demo_init(void)
{
    event_queue = xQueueCreateStatic(EVENT_QUEUE_LEN, sizeof(button_event_t),
                                     event_queue_storage, &event_queue_buf);
    button_init(on_button);
    xTaskCreateStatic(demo_task, "demo", DEMO_STACK_SIZE, NULL, DEMO_PRIORITY,
                      demo_stack, &demo_tcb);
}

void demo_tick_from_isr(void)
{
    button_tick_from_isr();
}
