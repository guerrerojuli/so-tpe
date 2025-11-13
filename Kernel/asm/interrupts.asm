GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL haltcpu
GLOBAL _hlt
GLOBAL _initialize_stack_frame


GLOBAL _exception0Handler, _exception6Handler

GLOBAL _irq00Handler
GLOBAL _irq01Handler

GLOBAL _int80Handler
GLOBAL _yieldHandler

GLOBAL _exception0Handler

EXTERN irqDispatcher
EXTERN intDispatcher
EXTERN exceptionDispatcher
EXTERN schedule

SECTION .text

%macro pushState 0
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rsi
	push rdi
	push rbp
	push rdx
	push rcx
	push rbx
	push rax
%endmacro

%macro popState 0
	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rbp
	pop rdi
	pop rsi
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
%endmacro

%macro popStateWithoutRax 0
	add rsp, 8
	pop rbx
	pop rcx
	pop rdx
	pop rbp
	pop rdi
	pop rsi
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
%endmacro

%macro irqHandlerMaster 1
	pushState

	mov rdi, %1
	mov rsi, rsp
	call irqDispatcher


	mov al, 20h
	out 20h, al

	popState
	iretq
%endmacro

%macro intHandlerMaster 0
	pushState
	mov rdi, rsp
	;sti
	call intDispatcher

	popStateWithoutRax
	iretq
%endmacro

%macro exceptionHandler 1
	pushState

	mov rdi, %1
	mov rsi, rsp
	call exceptionDispatcher

	popState
	iretq
%endmacro


_hlt:
	sti
	hlt
	ret

_cli:
	cli
	ret


_sti:
	sti
	ret

picMasterMask:
	push rbp
    mov rbp, rsp
    mov ax, di
    out	21h,al
    pop rbp
    retn

picSlaveMask:
	push    rbp
    mov     rbp, rsp
    mov     ax, di
    out	0A1h,al
    pop     rbp
    retn



_irq00Handler:
	pushState

	mov rdi, 0
	mov rsi, rsp
	call irqDispatcher


	mov rdi, rsp
	call schedule
	mov rsp, rax


	mov al, 20h
	out 20h, al

	popState
	iretq


_irq01Handler:
	irqHandlerMaster 1

_int80Handler:
	intHandlerMaster


_yieldHandler:
	pushState


	mov rdi, rsp
	call schedule
	mov rsp, rax

	popState
	iretq


_exception0Handler:
	exceptionHandler 0


_exception6Handler:
	exceptionHandler 6

haltcpu:
	cli
	hlt
	ret




_initialize_stack_frame:
	mov r8, rsp
	mov r9, rbp

	mov rsp, rdx
	mov rbp, rdx


	push 0x0
	push rdx
	push 0x202
	push 0x8
	push rdi


	mov rdi, rsi
	mov rsi, rcx
	pushState

	mov rax, rsp
	mov rsp, r8
	mov rbp, r9
	ret

SECTION .bss
	aux resq 1


section .note.GNU-stack noalloc noexec nowrite progbits
