#include "e1000.h"
#include <acpi/acpi.h>
#include <mem/pmm.h>
#include <klog/klog.h>
#include <panic.h>

// RX/TX ring pointers
uint8_t *rx_ptr;
uint8_t *tx_ptr;

// mac address
uint8_t mac_address[6];

// internals
uint64_t mmio_address;

// "internal" functions (not exposed via header)
static void read_mac_address();
static uint32_t read_command(uint16_t offset);
static void write_command(uint16_t address, uint32_t value);

void set_mac_address(uint8_t new_addr[6])
{
    uint32_t mac_low = 0;
    uint32_t mac_high = 0;
    mac_low += new_addr[0];
    mac_low += new_addr[1] << 8;
    mac_low += new_addr[2] << 16;
    mac_low += new_addr[3] << 24;
    mac_high += new_addr[4];
    mac_high += new_addr[5] << 8;

    write_command(0x5400, mac_low);
    write_command(0x5404, mac_high);
}

void e1000_init(uint64_t _mmio_address)
{
    rx_ptr = pmm_alloc(((sizeof(e1000_rx_desc) * E1000_NUM_RX_DESC + 16) / 0x1000) + 1);
    tx_ptr = pmm_alloc(((sizeof(e1000_tx_desc) * E1000_NUM_TX_DESC + 16) / 0x1000) + 1);

    mmio_address = _mmio_address;

    read_mac_address();

    klog("e1000", "MAC Address: %x:%x:%x:%x:%x:%x",
         mac_address[0],
         mac_address[1],
         mac_address[2],
         mac_address[3],
         mac_address[4],
         mac_address[5]);
}

void write_command(uint16_t address, uint32_t value)
{
    *((uint32_t *)(mmio_address + address)) = value;
}

uint32_t read_command(uint16_t offset)
{
    return *((uint32_t *)(mmio_address + offset));
}

void read_mac_address()
{
    uint32_t mac_low = read_command(0x5400);
    uint32_t mac_high = read_command(0x5404);
    mac_address[0] = mac_low & 0xff;
    mac_address[1] = mac_low >> 8 & 0xff;
    mac_address[2] = mac_low >> 16 & 0xff;
    mac_address[3] = mac_low >> 24 & 0xff;

    mac_address[4] = mac_high & 0xff;
    mac_address[5] = mac_high >> 8 & 0xff;
}
