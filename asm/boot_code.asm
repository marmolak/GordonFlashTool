bits 16
cpu 8086
org 0x7c00 ; boot code

start:
	jmp do
	nop
	times 32-($-$$) db 0
do:
    nop

    ; disable ray
    mov dx, 0x3D8
    in al, dx
    and al, 0x0F7
    out dx, al

    xor ax, ax
    mov ss, ax
    sti
    ; ax is still zero ^^^

	mov al, 0x03
	int 0x10

	; make background red
	mov ax, 0xb800
	mov es, ax
	mov ax, 0x4020
	mov cx, 80 * 25
	xor di, di
	rep stosw

	; write message
	mov di, 2000 - 24
	
	mov ah, 0xe0
    jmp text_addr 
r1:
    pop bx
    mov cx, 27
l1:
    cs mov al, byte [bx]
    inc bx
	stosw
    loop l1

    mov dx, 0x3D8
    in al, dx
    or al, 0x8
    out dx, al

h:  cli
	hlt ; waits for interrupts so we just ban them
    jmp h

text_addr:
    call r1
text:
    db ' Made by Gordon Flash Tool '

	;times 510-($-$$) db 0
	; This is our boot sector signature, without it,
	; the BIOS will not load our bootloader into memory.
	;dw 0xAA55
