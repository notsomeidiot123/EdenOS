#pragma once

#include <stdint.h>

#define COM_DATA 0
#define COM_INT 1
#define COM_FIFO 2
#define COM_LINE_CTRL 3
#define COM_MODEM_CTRL 4
#define COM_LINE_STATUS 5
#define COM_MODEM_STATUS 6
#define COM_SCRATCH 7
#define COM_DIV_LOW 0
#define COM_DIV_HIGH 1

void serial_init();
void serial_write(uint8_t port, uint8_t data);
uint8_t serial_read(uint8_t port);

uint8_t serial_read_config(uint8_t port, uint8_t reg);
void serial_write_config(uint8_t port, uint8_t reg, uint8_t data);