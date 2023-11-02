#!/bin/bash
qemu-system-x86_64 -d cpu_reset -smp cpus=1 -M q35 -m 2G -serial stdio -bios /usr/share/ovmf/OVMF.fd -cdrom cottage.iso -boot d
