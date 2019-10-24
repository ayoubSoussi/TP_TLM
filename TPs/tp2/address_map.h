#ifndef ADDRESS_MAP_H
#define ADDRESS_MAP_H

#define INST_RAM_SIZE (128 * 1024)
#define INST_RAM_BASEADDR 0x80000000

#define GPIO_BASEADDR 0x40000000
#define GPIO_SIZE     0x00010000
#include "hardware/offsets/gpio.h"

#define TIMER_BASEADDR 0x41C00000
#define TIMER_SIZE     0x00010000
#include "hardware/offsets/timer.h"

#define VGA_BASEADDR 0x73A00000
#define VGA_SIZE     0x00010000
#include "hardware/offsets/vga.h"

#define UART_BASEADDR 0x40600000
#define UART_SIZE     0x00010000
#include "hardware/offsets/uart.h"

#define INTC_BASEADDR 0x41200000
#define INTC_SIZE     0x00010000
#include "hardware/offsets/intc.h"

#endif
