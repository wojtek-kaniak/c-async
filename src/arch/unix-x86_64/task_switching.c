#include <assert.h>

#include "tasks.h"

[[gnu::sysv_abi]]
void async_task_trampoline(AsyncRuntime* runtime, void (*task)(uintptr_t), uintptr_t data)
{
	assert(runtime->current_task != ASYNC_NO_CURRENT_TASK);

	task(data);

	AsyncTask* current_task = async_find_task(runtime, runtime->current_task);
	Vector_AsyncTask_delete(&runtime->tasks, current_task - runtime->tasks.items);
	// TODO: free the stack

	async_switch_next(runtime, false);
	unreachable();
}
