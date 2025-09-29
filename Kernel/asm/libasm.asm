GLOBAL inb
GLOBAL outb

section .text
	
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