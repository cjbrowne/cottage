#pragma once

#include <stdint.h>

// NOTE: some QEMU versions reportedly use device 0x100E
// rather than 10D3 - they are similar devices, perhaps even compatible,
// but I've written the driver against my own QEMU version and cannot guarantee
// that it will work for others.  Modify DEVICE to 0x100E at your own risk.
#define E1000_PCI_VENDOR 0x8086
#define E1000_PCI_DEVICE 0x10D3
#define E1000_PCI_FUNCTN 0x0000

// the E1000 has 32 RX descriptors,
#define E1000_NUM_RX_DESC 32
#define E1000_NUM_TX_DESC 8

typedef struct {
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint16_t checksum;
    volatile uint8_t status;
    volatile uint8_t errors;
    volatile uint16_t special;
} __attribute__((packed)) e1000_rx_desc;

typedef struct {
    volatile uint64_t addr;
    volatile uint16_t length;
    volatile uint8_t cso;
    volatile uint8_t cmd;
    volatile uint8_t status;
    volatile uint8_t css;
    volatile uint16_t special;
} __attribute__((packed)) e1000_tx_desc;

void e1000_init(uint64_t mmio_address);
