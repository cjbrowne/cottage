//
// Created by chris on 03/10/23.
//

#include "serial.h"
#include <io/io.h>

int serial_init()
{
    outb(SERIAL_PORT_COM1 + 1, 0x00);    // Disable all interrupts
    outb(SERIAL_PORT_COM1 + 3, 0x80);    // Enable DLAB (set baud rate divisor)
    outb(SERIAL_PORT_COM1 + 0, 0x03);    // Set divisor to 3 (lo byte) 38400 baud
    outb(SERIAL_PORT_COM1 + 1, 0x00);    //                  (hi byte)
    outb(SERIAL_PORT_COM1 + 3, 0x03);    // 8 bits, no parity, one stop bit
    outb(SERIAL_PORT_COM1 + 2, 0xC7);    // Enable FIFO, clear them, with 14-byte threshold
    outb(SERIAL_PORT_COM1 + 4, 0x0B);    // IRQs enabled, RTS/DSR set
    outb(SERIAL_PORT_COM1 + 4, 0x1E);    // Set in loopback mode, test the serial chip
    outb(SERIAL_PORT_COM1 + 0, 0xAE);    // Test serial chip (send byte 0xAE and check if serial returns same byte)

    // Check if serial is faulty (i.e: not same byte as sent)
    if(inb(SERIAL_PORT_COM1 + 0) != 0xAE) {
        return 1;
    }

    // If serial is not faulty set it in normal operation mode
    // (not-loopback with IRQs enabled and OUT#1 and OUT#2 bits enabled)
    outb(SERIAL_PORT_COM1 + 4, 0x0F);
    return 0;
}

int is_transmit_empty() {
   return inb(SERIAL_PORT_COM1 + 5) & 0x20;
}

void write_serial(char a) {
   while (is_transmit_empty() == 0);

   outb(SERIAL_PORT_COM1,a);
}

void print_serial(const char* msg) {
    while(*msg) {
        write_serial(*msg);
        msg++;
    }
}