#include "task_switching.h"

void async_switch_task(CpuState* old_state, CpuState* target_state)
{
	async_switch_task_inner(old_state, target_state);
}

void async_switch_new_stack(AsyncRuntime* runtime, void (*task)(uintptr_t), uintptr_t data, void* stack_start, CpuState* old_state)
{
	async_switch_new_stack_inner(runtime, task, data, stack_start, old_state);
}
