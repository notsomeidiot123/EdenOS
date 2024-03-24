#pragma once
#include <stdint.h>

void kputc(char data);
void kputd(int32_t number);
void kputx(uint64_t number);
void kputs(char *s);
void kprintf(const char *fmt, ...);
void log_init(uint16_t port);