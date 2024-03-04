#pragma once
#include <stdint.h>

void putc(char data);
void putd(int32_t number);
void putx(uint64_t number);
void puts(char *s);
void printf(const char *fmt, ...);
void log_init(uint16_t port);