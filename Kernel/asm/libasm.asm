GLOBAL inb
GLOBAL outb
GLOBAL _xchg

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

; int _xchg (void *ptr, int value)
_xchg:
  mov rax, rsi
  xchg [rdi], eax
  ret

; Mark stack as non-executable for security (NX bit / DEP)
section .note.GNU-stack noalloc noexec nowrite progbits