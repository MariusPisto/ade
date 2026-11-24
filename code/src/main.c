#include <avr/interrupt.h>

#include "FreeRTOS.h"
#include "task.h"

#include "board.h"
#include "demo/demo.h"

int main(void)
{
    demo_init();
    vTaskStartScheduler();

    for (;;) {
    }
}

void vApplicationTickHook(void)
{
    demo_tick_from_isr();
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
