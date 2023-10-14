#pragma once

// conversion functions and macros for SI units

// converts KiB to bytes
#define KiB(x) (x * 1024)
#define MiB(x) (KiB(x) * 1024)
#define GiB(x) (MiB(x) * 1024)

// todo: make "bytes to human-readable string" function
