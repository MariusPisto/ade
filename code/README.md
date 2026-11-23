# ADE firmware (Arduino Uno + FreeRTOS)

Bare-metal C (no Arduino core) on top of FreeRTOS V11.1.0, as shipped in the
course resources (`resources_lab2_freeRTOS_2026`). The `GCC/ATMega328` port
ticks every 1 ms from Timer1 (compare match A).

## Setup

Toolchain: `avr-gcc` and `avrdude` from the Arduino IDE (found automatically
under `%LOCALAPPDATA%/Arduino15`), or from `PATH` on Linux/macOS. GNU `make`
must run from a POSIX shell (Git Bash on Windows).

## Build and flash

```sh
cd code
make                    # build/ade.hex + memory usage
make upload PORT=COM7   # /dev/ttyACM0 on Linux
make clean
```

## Layout

| Path | Content |
|---|---|
| `src/main.c` | Tasks, queues, FreeRTOS hooks |
| `src/board.h` | Pin map |
| `src/FreeRTOSConfig.h` | Kernel configuration (static allocation only) |
| `src/drivers/soft_i2c.*` | Bit-banged I2C master on D7 (SCL) / D8 (SDA) |
| `src/drivers/lcd.*` | Grove 16x2 LCD (JHD1802, 0x3E) on the software I2C bus |
| `src/drivers/button.*` | 3 Grove buttons on D4–D6, PCINT2 + debounce, events from ISR |
| `lib/FreeRTOS-Kernel` | FreeRTOS kernel and ATMega328 port from the course (only the files we build) |

## Current demo

The LCD shows the last button event on line 1 and the press count of each
button on line 2.
