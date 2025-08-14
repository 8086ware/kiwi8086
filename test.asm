bits 16
cpu 8086

global _start

_start:
	sti
	mov dx, 0x3d8
	mov al, 0b00001000
	out dx, al

	mov bx, 0
	mov es, bx

	mov es:[0x20], word irq0_handler
	mov es:[0x20 + 2], cs
	mov es:[0x24], word irq1_handler
	mov es:[0x24 + 2], cs
	mov bx, 0xb800
	mov es, bx

	xor bx, bx

	mov cx, 255
.loop_print_ascii:
	mov al, 2
	mul cl
	mov bx, ax
	mov es:[bx], cl
	mov es:[bx + 1], byte 0x12
	loop .loop_print_ascii

	; done

	jmp $
irq0_handler:
	mov es:[bx], al
	mov es:[bx + 1], al
	add bx, 2
	mov dx, 0x40
	xor ax, ax
	out dx, ax
	mov al, 0x20
	out 0x20, al
	iret
irq1_handler:
	in al, 0x60

	mov es:[bx], al
	mov es:[bx + 1], al
	add bx, 2
	mov al, 0x20
	out 0x20, al
	iret
_end:

times 510-(_end-_start) db 0
dw 0xaa55
