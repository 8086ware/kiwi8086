bits 16
cpu 8086

global _start

_start:
	;mov al, 0x4
	;mov ah, 0x0
	;int 0x10

	;mov ah, 0xb
	;mov bh, 0x1

	;mov bl, 1

	;int 0x10
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov al, 0b00001001
	mov dx, 0x3d8
	out dx, al

	mov al, 0b00100010
	mov dx, 0x3d9
	out dx, al

	xor ax, ax
.loop:
	mov bx, 0xb800
	mov es, bx
	mov di, 0
	inc ax
	mov cx, 80*25
	rep stosw

	jmp .loop
_end:

times 510-(_end-_start) db 0
dw 0xaa55
