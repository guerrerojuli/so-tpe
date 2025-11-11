GLOBAL inb
GLOBAL outb
GLOBAL _xchg

section .text
	

inb:
    mov dx, di
    in al, dx
    movzx eax, al
    ret


outb:
    mov dx, di
    mov al, sil
    out dx, al
    ret


_xchg:
  mov rax, rsi
  xchg [rdi], eax
  ret


section .note.GNU-stack noalloc noexec nowrite progbits