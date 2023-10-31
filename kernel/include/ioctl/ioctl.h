#pragma once

#include <stdint.h>
// IOCTL for TTY control
// nb: this has been ported from vinix and may need adjustment

#define IOCTL_TCGETS        0x5401
#define IOCTL_TCSETS        0x5402
#define IOCTL_TCSETSW       0x5403
#define IOCTL_TCSETSF       0x5404
#define IOCTL_TIOCSCTTY     0x540e
#define IOCTL_TIOCGWINSZ    0x5413

typedef struct {
    uint16_t ws_row;
    uint16_t ws_col;
    uint16_t ws_xpixel;
    uint16_t ws_ypixel;
} win_size_t;
