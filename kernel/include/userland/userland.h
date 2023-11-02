#pragma once

#include <fs/fs.h>
#include <cpu/smp.h>
#include <stdint.h>
#include <proc/proc.h>

// if replace == true, behaves like execve -- otherwise, behaves like fork
process_t* userland_start_program(
    bool replace, 
    vfs_node_t* cwd,
    const char* path,
    size_t argc,
    const char* argv[],
    size_t envc,
    const char* envp[],
    const char* stdin,
    const char* stdout,
    const char* stderr,
    size_t recursion_depth);
void userland_dispatch_signal(local_cpu_t* context);
