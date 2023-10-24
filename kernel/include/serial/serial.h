#pragma once

#define SERIAL_PORT_COM1 0x3f8

int serial_init();
void write_serial(char a);
void print_serial(const char *msg);
