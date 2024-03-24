#include "io.h"
uint8_t inb(uint16_t port){
    uint8_t byte = 0;
    asm volatile ("inb %1, %0" :  "=a"(byte) : "Nd"(port) );
    return byte;
}
void outb(uint16_t port, uint8_t byte){
    asm volatile ("outb %0, %1" ::  "a"(byte), "Nd"(port) );
}
void outw(uint16_t port, uint16_t data){
    asm volatile ("outw %0, %1" ::  "a"(data), "Nd"(port) );
}
uint16_t inw(uint16_t port){
    uint16_t word = 0;
    asm volatile ("inw %1, %0" :  "=a"(word) : "Nd"(port) );
    return word;
}