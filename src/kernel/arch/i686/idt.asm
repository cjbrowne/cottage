[bits 32]
; void __attribute__((cdecl)) i686_IDT_Load(IDT_descriptor* idt_desc);

global i686_IDT_Load
i686_IDT_Load:
    ; make new call frame
    push ebp             ; save old call frame
    mov ebp, esp         ; initialize new call frame

    ; load gdt
    mov eax, [ebp + 8]   ; eax = descriptor
    lidt [eax]

    ; restore old call frame
    mov esp, ebp
    pop ebp
    ret