GLOBAL cpuVendor
GLOBAL inb
GLOBAL outb

section .text
	
cpuVendor:
	push rbp
	mov rbp, rsp

	push rbx

	mov rax, 0
	cpuid


	mov [rdi], ebx
	mov [rdi + 4], edx
	mov [rdi + 8], ecx

	mov byte [rdi+13], 0

	mov rax, rdi

	pop rbx

	mov rsp, rbp
	pop rbp
	ret

; uint8_t inb(uint16_t port); 
inb:
    mov dx, di       ; puerto en rdi
    in al, dx        ; lee byte del puerto
    movzx eax, al    ; limpiar bits altos (eax = uint8_t)
    ret

; void outb(uint16_t port, uint8_t value)
outb:
    mov dx, di    ; puerto 
    mov al, sil   ; valor 
    out dx, al    ; enviar AL al puerto DX
    ret