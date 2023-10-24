#pragma once

#include <stdint.h>

typedef struct {
    char* header_name;
    char* header_value;
} http_header_t;

typedef struct {
    http_header_t* headers;
    uint8_t* body; 
} http_message_t;
