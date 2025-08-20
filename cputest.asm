bits 16
cpu 8086

global _start

;bh is the stage of testing

_start:
	mov bh, 0
	mov al, -5
	cbw
	cmp ax, -5
	jne cpu_fail

	mov bh, 1
	mov al, 127
	inc al
	jno cpu_fail

	mov ax, cs
	mov ds, ax

	mov bh, 2
	mov cx, 27
	mov si, string_test
	mov ax, 0x500 ; random memory location
	mov es, ax
	mov di, 0x500 ; random memory location
	cld
	rep movsb

	mov cx, 27
	mov si, string_test
	mov di, 0x500

	cld
	repe cmpsb
	jnz cpu_fail

	mov bh, 3
	mov al, -5
	dec al
	jns cpu_fail
	mov bh, 4
	mov ax, -5
	dec ax
	jns cpu_fail

	mov bh, 5
	
	mov al, 0b00000000
	inc al
	jp cpu_fail

	jmp $
string_test: db "This is an example string."
string_size: db 26
cpu_fail:
	mov al, bh
	out 0x80, al
	hlt
	jmp $-1
_end:

times 510-(_end-_start) db 0
dw 0xaa55
