org 0x7C00
bits 16

%define ENDL 0x0D, 0x0A

jmp short start
nop

;; FAT12 header

;; oem identifier
bdb_oem:                    db 'MSWIN4.1'
bdb_bytes_per_sector:       dw 512
bdb_sectors_per_cluster:    db 1
bdb_reserved_sectors:       dw 1
bdb_fat_count:              db 2
bdb_dir_entries_count:      dw 0E0h
bdb_total_sectors:          dw 2880
bdb_media_descriptor_type:  db 0F0h
bdb_sectors_per_fat:        dw 9
bdb_sectors_per_track:      dw 18
bdb_heads:                  dw 2
bdb_hidden_sectors:         dd 0
bdb_large_sector_count:     dd 0

ebr_drive_number:           db 0
                            db 0 ; reserved
ebr_signature:              db 29h
ebr_volume_id:              db 0DEh, 0ADh, 0BEh, 0EFh ; can be anything
ebr_volume_label:           db 'BASIC OS   '       ; must be 11 bytes
ebr_system_id:              db 'FAT12   '          ; must be 8 bytes

;;; code goes here ;;;

start:
    ; setup data segments
    mov ax, 0                   ; can't write to ds/es directly
    mov ds, ax
    mov es, ax

    ; setup stack
    mov ss, ax
    mov sp, 0x7C00

    ; some BIOSes start us at 07C0 0000 instead of 0000 7C00,
    ; this sequence makes sure we are in the location we expect.
    push es
    push word .after
    retf

.after:

    ; read something from floppy disk
    ; BIOS should set DL to drive number
    mov [ebr_drive_number], dl

    ; show loading message
    mov si, msg_loading
    call puts

    ; read drive parameters
    push es
    mov ah, 08h
    int 13h
    jc floppy_error
    pop es

    and cl, 3Fh                        ; remove top 2 bits from CL
    xor ch, ch
    mov [bdb_sectors_per_track], cx     ; sector count

    inc dh
    mov [bdb_heads], dh                 ; head count

    ; compute LBA of root directory
    mov ax, [bdb_sectors_per_fat]
    mov bl, [bdb_fat_count]
    xor bh, bh
    mul bx                              ; dx:ax = (fats * sectors_per_fat)
    add ax, [bdb_reserved_sectors]
    push ax

    ; compute size of root directory
    mov ax, [bdb_dir_entries_count]
    shl ax, 5           ; ax *= 32
    xor dx, dx
    div word [bdb_bytes_per_sector] ; number of sectors we need to read

    test dx, dx             ; if dx != 0; dx += 1
    jz .root_dir_after
    inc ax

.root_dir_after:

    ; read root directory
    mov cl, al                      ; number of sectors to read
    pop ax
    mov dl, [ebr_drive_number]
    mov bx, buffer
    call disk_read

    ; search for stage2.bin
    xor bx,bx
    mov di, buffer

.search_kernel:
    mov si, file_stage2_bin
    mov cx, 11
    push di
    repe cmpsb              ; compare string byte
    pop di
    je .found_kernel

    add di, 32
    inc bx
    cmp bx, [bdb_dir_entries_count]
    jl .search_kernel

    ; stage2 not found
    jmp kernel_not_found_error

.found_kernel:

    ; di should have the address to the entry
    mov ax, [di + 26]                       ; first logical cluster field
    mov [stage2_cluster], ax

    ; load FAT from disk into memory
    mov ax, [bdb_reserved_sectors]
    mov bx, buffer
    mov cl, [bdb_sectors_per_fat]
    mov dl, [ebr_drive_number]
    call disk_read

    ; read stage2 and process FAT chain
    mov bx, STAGE2_LOAD_SEGMENT
    mov es, bx
    mov bx, STAGE2_LOAD_OFFSET

.load_kernel_loop:
    mov ax, [stage2_cluster]
    ; 31 should not be hard-coded here (only works for floppies)
    add ax, 31

    mov cl, 1
    mov dl, [ebr_drive_number]
    call disk_read

    add bx, [bdb_bytes_per_sector]

    ; compute location of next cluster
    mov ax, [stage2_cluster]
    mov cx, 3
    mul cx
    mov cx, 2
    div cx

    mov si, buffer
    add si, ax
    mov ax, [ds:si]

    or dx, dx
    jz .even

.odd:
    shr ax, 4
    jmp .next_cluster_after
.even:
    and ax, 0x0FFF

.next_cluster_after:
    cmp ax, 0x0FF8              ; end of chain
    jae .read_finish

    mov [stage2_cluster], ax
    jmp .load_kernel_loop

.read_finish:
    mov dl, [ebr_drive_number] ; boot device in dl

    mov ax, STAGE2_LOAD_SEGMENT ; set segment registers
    mov ds, ax
    mov es, ax

    jmp STAGE2_LOAD_SEGMENT:STAGE2_LOAD_OFFSET

    ; failsafe for if the stage2 jump somehow fails
    jmp wait_key_and_reboot

    ; failsafe halt
    cli
    hlt

;;; error handlers ;;;
floppy_error:
    mov si, msg_read_failed
    call puts
    jmp wait_key_and_reboot

kernel_not_found_error:
    mov si, msg_stage2_not_found
    call puts
    jmp wait_key_and_reboot

wait_key_and_reboot:
    mov ah, 0
    int 0x16            ; wait for keypress
    jmp 0xffff:0        ; jump to beginning of BIOS, should reboot


; failsafe HALT
.halt:
    cli                 ; disable interrupts so we can't get out of "HALT"
    hlt                 ; HALT


;;; helper routines ;;;

; Prints a string to the screen.
; Params:
;   - ds:si points to string
puts:
    push si
    push ax
    push bx

.loop:
    lodsb
    or al,al
    jz .done

    ; call bios interrupt to print character
    mov ah, 0x0e
    mov bh, 0
    int 0x10

    jmp .loop
.done:
    pop bx
    pop ax
    pop si
    ret

;
; Disk routines
;

;
; Converts an LBA address to a CHS address
; Parameters:
;   - ax: LBA address
; Returns:
;   - cx [bits 0-5]: sector number
;   - cx [bits 6-15]: cylinder
;   - dh: head
;
lba_to_chs:
    push ax
    push dx

    xor dx, dx      ; dx = 0
    div word [bdb_sectors_per_track]    ; dx = LBA /  SectorsPerTrack
                                        ; dx = LBA % SectorsPerTrack
    inc dx                              ; dx = (LBA % SectorsPerTrack + 1)
    mov cx, dx                          ; cx = sector

    xor dx,dx               ; reset dx
    div word [bdb_heads]    ; ax = (LBA / SectorsPerTrack) / Heads = cylinder
                            ; dx = (LBA / SectorsPerTrack) % Heads = head
                            ; dl = head
    mov dh, dl              ; dh = head
    mov ch, al              ; ch = cylinder (lower 8 bits)
    shl ah, 6
    or cl, ah               ; put upper 2 bits of cylinder in CL

    ; inexplicably only restore DL
    ; idk why the tutorial decided to not just pop dx, pop ax
    ; but I'm sure he has his reasons...
    pop ax
    mov dl, al
    pop ax
    ret

; Reads from disk
; ax - LBA address
; cl - number of sectors to read (up to 128)
; dl - drive number
; es:bx - memory adress where to store read data
disk_read:
    ; save all registers that might be affected
    push ax
    push bx
    push cx
    push dx
    push di


    push cx             ; save CX
    call lba_to_chs     ; compute CHS
    pop ax              ; AL = number of sectors to read

    mov ah, 02h         ; set up for interrupt 13
    mov di, 3         ; retry count

.retry:
    pusha
    stc                 ; set carry flag (some bios' don't set it)
    int 13h            ; call BIOS interrupt "Read Disk Sectors"
    jnc .done

    ; on failure
    popa
    call disk_reset

    dec di
    test di, di
    jnz .retry

.fail:
    ; 3 times failed to load from floppy
    jmp floppy_error

.done:
    popa
    ; restore registers we saved at the start
    pop di
    pop dx
    pop cx
    pop bx
    pop ax
    ret

;
; Resets disk controller
; Parameters:
;   dl: drive number
;
disk_reset:
    pusha
    mov ah, 0
    stc
    int 13h
    jc floppy_error
    popa
    ret

msg_loading:            db 'Loading...', ENDL, 0
msg_read_failed:        db 'Read from disk failed!', ENDL, 0
msg_stage2_not_found:   db 'STAGE2.BIN file not found!', ENDL, 0
file_stage2_bin:        db 'STAGE2  BIN'
stage2_cluster:         dw 0

STAGE2_LOAD_SEGMENT     equ 0x0
STAGE2_LOAD_OFFSET      equ 0x500

; zero-pad the end
times 510-($-$$) db 0
dw 0AA55h ; BIOS boot signature (required)

buffer: