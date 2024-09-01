; Task switching overview:
; - store the CPU state
; - load the target CPU state
; - return (using the target stack)

global async_switch_task_inner, async_switch_new_stack_inner

; void async_switch_task_inner(CpuState* old, CpuState* target)
async_switch_task_inner:
	cmp rdi, 0
	jz load_task_state
	mov [rdi],		rbx
	mov [rdi + 8],	rsp
	mov [rdi + 16],	rbp
	mov [rdi + 24],	r12
	mov [rdi + 32],	r13
	mov [rdi + 40],	r14
	mov [rdi + 48],	r15

load_task_state:
	mov rbx, [rsi]
	mov rsp, [rsi + 8]
	mov rbp, [rsi + 16]
	mov r12, [rsi + 24]
	mov r13, [rsi + 32]
	mov r14, [rsi + 40]
	mov r15, [rsi + 48]

	ret

extern async_task_trampoline

; void async_switch_new_stack_inner(AsyncRuntime* runtime, void (*task)(void*), void* data, void* stack, CpuState* old)
async_switch_new_stack_inner:
	cmp r8,	0
	jz load_new_task_state
	mov [r8],		rbx
	mov [r8 + 8],	rsp
	mov [r8 + 16],	rbp
	mov [r8 + 24],	r12
	mov [r8 + 32],	r13
	mov [r8 + 40],	r14
	mov [r8 + 48],	r15
load_new_task_state:
	; align the stack to a 16 byte boundary
	and rcx, -16
	sub rcx, 8
	mov rsp, rcx
	xor rcx, rcx
	jmp async_task_trampoline
