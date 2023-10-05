#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    const char* name;
    bool (*probe)();
    void (*init)(uint8_t offset_pic1, uint8_t offset_pic2, bool autoEOI);
    void (*disable)();
    void (*sendEndOfInterrupt)(int irq);
    void (*mask)(int irq);
    void (*unmask)(int irq);
} PICDriver;