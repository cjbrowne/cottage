#include "pci.h"
#include <stdint.h>
#include <io/io.h>

#define COMMAND_PORT 0xcf8
#define DATA_PORT 0xcfc

uint8_t pci_readb(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset)
{
    uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);

	outd(COMMAND_PORT, id);
	uint8_t result = inb(DATA_PORT);
	return result >> (8 * (registeroffset % 4));
}

uint16_t pci_readw(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset)
{
    uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
    
	outd(COMMAND_PORT, id);
	uint16_t result = inw(DATA_PORT);
	return result >> (8 * (registeroffset % 4));
}

uint32_t pci_readd(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset)
{
    uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
    
	outd(COMMAND_PORT, id);
	uint32_t result = ind(DATA_PORT);
	return result >> (8 * (registeroffset % 4));
}

void pci_writeb(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint8_t val)
{
    uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
    outd(COMMAND_PORT, id);
	outb(DATA_PORT, val);
}

void pci_writew(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint16_t val)
{
    uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
    outd(COMMAND_PORT, id);
	outw(DATA_PORT, val);
}

void pci_writed(uint16_t bus, uint16_t device, uint16_t function, uint32_t registeroffset, uint32_t val)
{
    uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (registeroffset & 0xFC);
    outd(COMMAND_PORT, id);
	outd(DATA_PORT, val);
}
