#if !defined (INCL_TASK_SWITCHING_U)
#define INCL_TASK_SWITCHING_U

#if defined (__unix__) && defined (__x86_64__)
// IWYU pragma: begin_exports
#include "unix-x86_64/cpu.h"
#include "unix-x86_64/task_switching.h"
// IWYU pragma: end_exports
#else
#error C-Async: unsupported architecture
#endif

#include "tasks.h"

/// `old_state` may be null, in which case the old state won't be saved
void async_switch_task(CpuState* old_state, CpuState* target_state);

/// `old_state` may be null, in which case the old state won't be saved
void async_switch_new_stack(AsyncRuntime* runtime, void (*task)(uintptr_t), uintptr_t data, void* stack_start, CpuState* old_state);

#endif
