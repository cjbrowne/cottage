#pragma once

#include <stdint.h>

uint8_t pci_readb(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset);
uint16_t pci_readw(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset);
uint32_t pci_readd(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset);
void pci_writeb(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint8_t val);
void pci_writew(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint16_t val);
void pci_writed(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t val);
