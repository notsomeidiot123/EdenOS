#include <stdint.h>
#include "io.h"
#include "serial.h"

uint16_t serial_ports[8] = {0};

extern void test_eax(uint32_t value);

void serial_init() {
    uint16_t *com_ports = (void *)0x400;
    for(int i = 0; i < 4; i++){
        outb(com_ports[i] + COM_INT, 0);
        outb(com_ports[i] + COM_LINE_CTRL, 0x80); //enable dlab
        outb(com_ports[i] + COM_DIV_LOW, 3);
        outb(com_ports[i] + COM_DIV_HIGH, 0);
        outb(com_ports[i] + COM_LINE_CTRL, 3);//8 bits no parity, 1 stop
        outb(com_ports[i] + COM_FIFO, 0xc7);//enable fifo & clear
        outb(com_ports[i] + COM_MODEM_CTRL, 0x0b);
        outb(com_ports[i] + COM_MODEM_CTRL, 0x1E); //loopback for test
        outb(com_ports[i] + COM_DATA, 0xa5);
        uint8_t test = inb(com_ports[i] + COM_DATA);
        if(test == 0xA5){
            outb(com_ports[i] + COM_MODEM_CTRL, 0x0f);
            serial_ports[i] = com_ports[i];
        }
    }
    
}
void serial_write(uint8_t port, uint8_t data){
    if(port > 8 || !serial_ports[port]){
        return;
    }
    outb(serial_ports[port], data);
}
uint8_t serial_read(uint8_t port){
    if(port > 8 || !serial_ports[port]){
        return 0;
    }
    return inb(serial_ports[port]);
}

uint8_t serial_read_config(uint8_t port, uint8_t reg) {
    if(port > 8 || !serial_ports[port] || reg > 7){
        return -1;
    }
    return inb(serial_ports[port] + reg);
}
void serial_write_config(uint8_t port, uint8_t reg, uint8_t data){
    if(port > 8 || !serial_ports[port] || reg > 7){
        return;
    }
    outb(serial_ports[port] + reg, data);
}