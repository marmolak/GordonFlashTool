bits 16
org 7c00h ; boot code
; dos binary org 100h

; 0x000b8000 -> color
; 14 - yellow
; 4 - red

start:
	jmp do
	nop
	times 32-($-$$) db 0
do:
    nop
    mov ax, 0
    mov ss, ax
    sti
	xor ax, ax
	mov al, 03h
	int 10h

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
