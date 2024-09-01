#if !defined (INCL_ARCH_UNIX_X86_64_TASK_SWITCHING_H)
#define INCL_ARCH_UNIX_X86_64_TASK_SWITCHING_H

#include "tasks.h"
#include "arch/unix-x86_64/cpu.h"

/// `old_state` may be null, in which case the old state won't be saved
[[gnu::sysv_abi]]
extern void async_switch_task_inner(CpuState* old_state, CpuState* target_state);


[[gnu::sysv_abi]]
void async_switch_new_stack_inner(AsyncRuntime* runtime, void (*task)(uintptr_t), uintptr_t data, void* stack_start, CpuState* old_state);

#endif
