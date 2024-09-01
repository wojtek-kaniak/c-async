#include <assert.h>

#include "async.h"
#include "tasks.h"

void async_scheduler_enqueue(AsyncRuntime* runtime, AsyncTaskId task)
{
	assert(runtime_id(task) == runtime->id);

	for (size_t task_ix = 0; task_ix < runtime->ready_queue.length; task_ix++)
	{
		// prevent double enqueue
		if (runtime->ready_queue.items[task_ix] == task)
			return;
	}

	Vector_AsyncTaskId_add(&runtime->ready_queue, runtime->options.alloc_options, task);
}

AsyncTaskId async_scheduler_next(AsyncRuntime* runtime)
{
	if (runtime->ready_queue.length == 0)
		async_panic("no ready tasks");
	
	return Vector_AsyncTaskId_delete(&runtime->ready_queue, 0);
}
