#ifndef DEMO_H
#define DEMO_H

#include <stdint.h>

/* One driver test program per file, picked at build time: make DEMO=<name>. */

void demo_init(void);           /* before the scheduler: drivers and tasks */
void demo_tick_from_isr(void);  /* from the tick hook */

/* Writes v right-aligned, its last digit just before `end`. */
static inline void demo_put_u16(char *end, uint16_t v)
{
    do {
        *--end = '0' + v % 10;
        v /= 10;
    } while (v);
}

#endif
