#include <stdint.h>
#include "log.h"
#include "../hardware/serial.h"

#include <stdarg.h>

uint8_t log_port = 0;

void log_init(uint16_t port){
    log_port = port;
}

void putc(char c){
    serial_write(log_port, c);
}
void putd(int32_t number){
    if(number < 0){
        serial_write(log_port, '-');
        number *= -1;
    }
    char buffer[40] = {0};
    uint8_t index = 1;
    while(number > 0){
        buffer[index++] = (number % 10) + '0';
        number /= 10;
    }
    while(index > 0){
        serial_write(log_port, buffer[index--]);
    }
}
void putx(uint64_t number){
    char buffer[40] = {0};
    uint8_t index = 1;
    while(number > 0){
        buffer[index] = (number % 16); 
        buffer[index] > 9 ? (buffer[index] += 'A' - 10) : (buffer[index] += '0');
        index++;
        number /= 16;
    }
    while(index > 0){
        serial_write(log_port, buffer[index--]);
    }
}
void padded_putx(uint32_t number){
    char buffer[40] = {0};
    uint8_t index = 1;
    while(number > 0){
        buffer[index] = (number % 16); 
        buffer[index] > 9 ? (buffer[index] += 'A' - 10) : (buffer[index] += '0');
        index++;
        number /= 16;
    }
    for(int i = 0; i < 8 - (index -1); i++){
        serial_write(log_port, '0');
    }
    while(index > 0){
        serial_write(log_port, buffer[index--]);
    }
}
void puts(char *s){
    while(*s){
        serial_write(log_port, *(s++));
    }
}

void printf(const char *fmt, ...){
    va_list args;
    va_start(args, fmt);
    int64_t arg = 0;
    while(*fmt){
        if(*fmt == '%'){
            fmt++;
            switch(*fmt){
                case 'c':
                    arg = va_arg(args, int32_t);
                    putc(arg);
                    break;
                case 's':
                    arg = (uint64_t)va_arg(args, void*);
                    puts((char *)arg);
                    break;
                case 'd':
                    arg = va_arg(args, int32_t);
                    putd(arg);
                    break;
                case 'l':
                    arg = va_arg(args, int64_t);
                    putd(arg);
                    break;
                case 'x':
                    arg = va_arg(args, int32_t);
                    putx(arg);
                    break;
                case '0':
                    fmt++;
                    if(*fmt == 'x'){
                        arg = va_arg(args, int64_t);
                        padded_putx(arg);
                    }
                    else{
                        fmt--;
                    }
                    break;
                default:
                    serial_write(log_port, '%');
                    serial_write(log_port, *fmt);
                    break;
            }
            fmt++;
            continue;
        }
        serial_write(log_port, *fmt);
        fmt++;
    }
    return;
}