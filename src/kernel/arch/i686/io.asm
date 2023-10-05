
global i686_outb
i686_outb:
    [bits 32]
    mov dx, [esp + 4]
    mov al, [esp + 8]
    out dx, al
    ret

global i686_inb
i686_inb:
    [bits 32]
    mov dx, [esp + 4]
    xor eax, eax
    in al, dx
    ret


global i686_EnableInterrupts
i686_EnableInterrupts:
    sti
    ret


global i686_DisableInterrupts
i686_DisableInterrupts:
    cli
    ret

global i686_Panic
i686_Panic:
    cli
    hlt

global crash_me
crash_me:
    ; first set ecx=0xdead, edx=0xbeef, so we can identify the purposeful crash
    mov ecx, 0xdead
    mov edx, 0xbeef

    ; call an interrupt that doesn't exist
    ;int 0x50

    ; divide by zero
    mov eax, 0
    div eax

    ret