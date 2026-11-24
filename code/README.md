# ADE firmware (Arduino Uno + FreeRTOS)

Bare-metal C (no Arduino core) on top of FreeRTOS V11.1.0, as shipped in the
course resources (`resources_lab2_freeRTOS_2026`). The `GCC/ATMega328` port
ticks every 1 ms from Timer1 (compare match A).

## Setup

Toolchain: `avr-gcc` and `avrdude` from the Arduino IDE (found automatically
under `%LOCALAPPDATA%/Arduino15`), or from `PATH` on Linux/macOS. GNU `make`
must run from a POSIX shell (Git Bash on Windows).

## Build and flash

Each driver has a test program in `src/demo/`, picked with `DEMO`
(default `light`):

```sh
cd code
make DEMO=light                      # build/light/ade.hex + memory usage
make DEMO=light upload PORT=COM7     # /dev/ttyACM0 on Linux
make clean
```

| `DEMO` | Wiring (LCD always on D7) | LCD shows |
|---|---|---|
| `buttons` | buttons on D4, D5, D6 | last event on line 1, press count per button on line 2 |
| `ultrasonic` | ultrasonic ranger on D2 | distance on line 1; measurements per second (50 expected) and lost echoes on line 2 |
| `light` | light sensor on A1 | raw and smoothed values on line 1, bar graph of the level on line 2 |

## Layout

| Path | Content |
|---|---|
| `src/main.c` | Entry point, FreeRTOS hooks, fatal error handler |
| `src/board.h` | Pin map |
| `src/FreeRTOSConfig.h` | Kernel configuration (static allocation only) |
| `src/demo/` | One driver test program per file |
| `src/drivers/soft_i2c.*` | Bit-banged I2C master on D7 (SCL) / D8 (SDA) |
| `src/drivers/lcd.*` | Grove 16x2 LCD (JHD1802, 0x3E) on the software I2C bus |
| `src/drivers/button.*` | 3 Grove buttons on D4–D6, PCINT2 + debounce, events from ISR |
| `src/drivers/ultrasonic.*` | Grove ultrasonic ranger on D2: ping every 20 ms from the tick hook, echo timed by INT0 (4 µs) |
| `src/drivers/adc.*` | 10-bit ADC, AVCC reference, blocking read |
| `src/drivers/light.*` | Grove light sensor v1.2 on A1, smoothed by a moving average |
| `lib/FreeRTOS-Kernel` | FreeRTOS kernel and ATMega328 port from the course (only the files we build) |
