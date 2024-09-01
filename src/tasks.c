#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include <threads.h>

#include "async.h"
#include "common.h"
#include "tasks.h"
#include "scheduler.h"
#include "arch/stack.h"
#include "arch/task_switching.h"

[[noreturn]] static void default_async_alloc_fail_handler()
{
	async_panic("allocation failure");
}

AsyncRuntimeOptions ASYNC_DEFAULT_RUNTIME_OPTS = {
	// 1 MiB
	.stack_size = 1024 * 1024,
	.alloc_options = {
		.malloc = malloc,
		.realloc = realloc,
		.free = free,
		.async_alloc_fail = default_async_alloc_fail_handler,
	},
};

static _Atomic(uint32_t) last_runtime_id = 0;

static thread_local bool thread_runtime_initialized = false;
static thread_local AsyncRuntime thread_runtime;

static AsyncRuntime* get_thread_runtime()
{
	if (!thread_runtime_initialized)
		async_panic("async runtime not initialized, `async_runtime_create` must be called first");

	return &thread_runtime;
}

void async_runtime_create_with_opts(AsyncRuntimeOptions options)
{
	if (thread_runtime_initialized)
		async_panic("async runtime already initialized");

	uint32_t id = last_runtime_id++;

	thread_runtime = (AsyncRuntime) {
		.id = id,
		.options = options,
		.tasks = Vector_AsyncTask_create(options.alloc_options, 0),
		.ready_queue = Vector_AsyncTaskId_create(options.alloc_options, 0),
		.next_task_id = 0,
		.current_task = ASYNC_NO_CURRENT_TASK,
	};

	thread_runtime_initialized = true;
}

void async_runtime_create()
{
	async_runtime_create_with_opts(ASYNC_DEFAULT_RUNTIME_OPTS);
}

void async_runtime_free()
{
	AsyncRuntime* runtime = get_thread_runtime();

	if (runtime->tasks.length > 0)
		async_panic("runtime still has unfinished tasks");

	AsyncAllocOptions alloc_opts = runtime->options.alloc_options;

	Vector_AsyncTask_free(&runtime->tasks, alloc_opts);

	// TODO: free tasks

	thread_runtime_initialized = false;
}

AsyncTask* async_find_task(AsyncRuntime* runtime, AsyncTaskId id)
{
	assert(runtime_id(id) == runtime->id);

	// TODO: map
	for (size_t i = 0; i < runtime->tasks.length; i++)
	{
		if (runtime->tasks.items[i].id == id)
			return &runtime->tasks.items[i];
	}

	return nullptr;
}

static bool should_exit(AsyncRuntime* runtime)
{
	for (size_t i = 0; i < runtime->tasks.length; i++)
	{
		if ((runtime->tasks.items[i].flags & ASYNC_TASK_BACKGROUND) == 0)
			return false;
	}

	return true;
}

// `store_current_state` - false if the current task has finished
void async_switch_next(AsyncRuntime* runtime, bool store_current_state)
{
	CpuState* current_cpu_state;
	if (store_current_state)
	{
		if (runtime->current_task == ASYNC_NO_CURRENT_TASK)
		{
			// startup:
			async_log("switching from the start state");

			if (should_exit(runtime))
			{
				async_log("no tasks to switch to");
				return;
			}

			current_cpu_state = &runtime->start_state;
		}
		else
		{
			AsyncTask* current_task = async_find_task(runtime, runtime->current_task);
			assert(current_task != nullptr);

			async_log("switching from a task (id: %" PRIx64 ")", runtime->current_task);

			current_cpu_state = &current_task->last_cpu_state;
		}
	}
	else
		current_cpu_state = nullptr;

	if (should_exit(runtime))
	{
		async_log("runtime exiting, switching to the start state");

		runtime->current_task = ASYNC_NO_CURRENT_TASK;
		async_switch_task(current_cpu_state, &runtime->start_state);
	}
	else
	{
		AsyncTaskId task_id = async_scheduler_next(runtime);
		AsyncTask* target_task = async_find_task(runtime, task_id);

		if (target_task == nullptr)
			async_panic("task returned by the scheduler not found");

		runtime->current_task = task_id;

		if (target_task->func == nullptr)
		{
			async_log("switching to a task (id: %" PRIx64 ")", task_id);

			CpuState* target_cpu_state = &target_task->last_cpu_state;
			async_switch_task(current_cpu_state, target_cpu_state);
		}
		else
		{
			async_log("switching to a new task (id: %" PRIx64 ")", task_id);

			void(*func)(uintptr_t) = target_task->func;
			target_task->func = nullptr;

			// task being run for the first time:
			async_switch_new_stack(
				runtime,
				func,
				target_task->func_data,
				target_task->stack.start_address,
				current_cpu_state
			);
		}
	}
}

static AsyncTask* get_current_task(AsyncRuntime* runtime)
{
	if (runtime->current_task == ASYNC_NO_CURRENT_TASK)
		async_panic("async function called without a runtime");

	assert(runtime_id(runtime->current_task) == runtime->id);
	
	AsyncTask* current_task = async_find_task(runtime, runtime->current_task);
	assert(current_task != nullptr);

	return current_task;
}

void async_yield()
{
	AsyncRuntime* runtime = get_thread_runtime();
	
	get_current_task(runtime);

	async_scheduler_enqueue(runtime, runtime->current_task);

	async_switch_next(runtime, true);
}

void async_suspend(AsyncRuntime* runtime)
{
	get_current_task(runtime);

	async_switch_next(runtime, true);
}

void async_resume(AsyncRuntime* runtime, AsyncTaskId task)
{
	if (runtime_id(task) != runtime->id)
		async_panic("async_resume: `task` belongs to a different runtime");

	async_scheduler_enqueue(runtime, task);
}

AsyncTaskId async_current_task()
{
	AsyncRuntime* runtime = get_thread_runtime();

	/// assert there is a task currently running:
	get_current_task(runtime);

	return runtime->current_task;
}

AsyncTaskOptions ASYNC_DEFAULT_TASK_OPTS = {
	.flags = 0,
	.stack_size = 0,
};

AsyncTaskId async_spawn(void (*task)(uintptr_t), uintptr_t data)
{
	return async_spawn_with_opts(task, data, ASYNC_DEFAULT_TASK_OPTS);
}

AsyncTaskId async_spawn_with_opts(void (*task)(uintptr_t), uintptr_t data, AsyncTaskOptions options)
{
	assert(task != nullptr);

	AsyncRuntime* runtime = get_thread_runtime();

	size_t requested_stack_size = options.stack_size == 0
		? runtime->options.stack_size
		: options.stack_size;

	Stack stack = async_create_stack(requested_stack_size);

	uint32_t local_id = runtime->next_task_id++;
	AsyncTaskId id = task_id(runtime->id, local_id);

	AsyncTask new_task = {
		.id = id,
		.flags = options.flags,
		.stack = stack,
		.func = task,
		.func_data = data,
	};

	Vector_AsyncTask_add(&runtime->tasks, runtime->options.alloc_options, new_task);

	async_scheduler_enqueue(runtime, id);

	return id;
}

void async_runtime_start()
{
	AsyncRuntime* runtime = get_thread_runtime();

	if (runtime->current_task != ASYNC_NO_CURRENT_TASK)
		async_panic("runtime already started");
	
	async_switch_next(runtime, true);
}
