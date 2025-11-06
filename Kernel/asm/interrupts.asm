GLOBAL _cli
GLOBAL _sti
GLOBAL picMasterMask
GLOBAL picSlaveMask
GLOBAL haltcpu
GLOBAL _hlt
GLOBAL _initialize_stack_frame

; Excepcion div zero y invalid opcode
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

	mov rdi, %1 ; pasaje de parametro (número de IRQ)
	mov rsi, rsp ; pasaje del puntero a los registros
	call irqDispatcher

	; signal pic EOI (End of Interrupt)
	mov al, 20h
	out 20h, al

	popState
	iretq
%endmacro

%macro intHandlerMaster 0
	pushState
	mov rdi, rsp    ; Pasa puntero a toda la estructura
	sti
	call intDispatcher

	popStateWithoutRax
	iretq
%endmacro

%macro exceptionHandler 1
	pushState

	mov rdi, %1 ; pasaje de parametro
	mov rsi, rsp ; paso todos los registros
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
    mov     ax, di  ; ax = mascara de 16 bits
    out	0A1h,al
    pop     rbp
    retn


;8254 Timer (Timer Tick) - with context switch
_irq00Handler:
	pushState

	mov rdi, 0
	mov rsi, rsp
	call irqDispatcher

	; Context switch
	mov rdi, rsp       ; Pass current RSP to scheduler
	call schedule      ; Returns new RSP in RAX
	mov rsp, rax       ; Switch to new process stack

	; Signal PIC EOI
	mov al, 20h
	out 20h, al

	popState
	iretq

;Keyboard
_irq01Handler:
	irqHandlerMaster 1

_int80Handler:
	intHandlerMaster

; Yield handler - same as timer but without incrementing ticks
_yieldHandler:
	pushState

	; Context switch only (no timer tick)
	mov rdi, rsp       ; Pass current RSP to scheduler
	call schedule      ; Returns new RSP in RAX
	mov rsp, rax       ; Switch to new process stack

	popState
	iretq

;Zero Division Exception
_exception0Handler:
	exceptionHandler 0

;Invalid Op Code Exception
_exception6Handler:
	exceptionHandler 6

haltcpu:
	cli
	hlt
	ret

; Initialize stack frame for new process
; Args: RDI = wrapper, RSI = entry point, RDX = stack top, RCX = args
; Returns: RAX = new stack pointer
_initialize_stack_frame:
	mov r8, rsp         ; Save current RSP
	mov r9, rbp         ; Save current RBP

	mov rsp, rdx        ; Switch to process stack
	mov rbp, rdx

	; Setup iretq frame
	push 0x0            ; SS
	push rdx            ; RSP
	push 0x202          ; RFLAGS (IF=1)
	push 0x8            ; CS
	push rdi            ; RIP (wrapper function)

	; Setup register state
	mov rdi, rsi        ; First arg to wrapper (entry point)
	mov rsi, rcx        ; Second arg to wrapper (args)
	pushState           ; Push all 15 registers

	mov rax, rsp        ; Return new stack position
	mov rsp, r8         ; Restore original RSP
	mov rbp, r9         ; Restore original RBP
	ret

SECTION .bss
	aux resq 1