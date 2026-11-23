#ifndef SOFT_I2C_H
#define SOFT_I2C_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Bit-banged I2C master (~45 kHz) on the pins of board.h.
 * Interrupts stay enabled: preemption only stretches the clock.
 * Not thread-safe: a single task owns the bus.
 */

void si2c_init(void);
bool si2c_start(uint8_t addr, bool read);  /* (repeated) START + address, true on ACK */
bool si2c_write(uint8_t byte);             /* true on ACK */
uint8_t si2c_read(bool ack);
void si2c_stop(void);
bool si2c_probe(uint8_t addr);

#endif
