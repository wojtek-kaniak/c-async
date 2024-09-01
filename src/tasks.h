#if !defined(INCL_TASKS_H)
#define INCL_TASKS_H

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

typedef struct AsyncRuntime AsyncRuntime;

#include "async.h"
#include "arch/stack.h"
#include "arch/task_switching.h"

typedef struct AsyncTask {
	AsyncTaskId id;

	Stack stack;

	/// arch-specific CPU state of the task at the time of last suspension
	CpuState last_cpu_state;

	AsyncTaskFlags flags;

	/// non-null before the task is started
	void (*func)(uintptr_t);
	uintptr_t func_data;

	/// TODO: post finish actions to implement tasks waiting for other tasks
} AsyncTask;

/// Task ID unique to a runtime, combined with a runtime ID forms a `AsyncTaskId`
typedef uint32_t LocalAsyncTaskId;

// TODO: a better data structure (a map?)
#define TYPE AsyncTask
#include "generic/vector.h"
#undef TYPE

// TODO: use a deque instead
#define TYPE AsyncTaskId
#include "generic/vector.h"
#undef TYPE

#define ASYNC_NO_CURRENT_TASK ((AsyncTaskId) -1)

typedef struct AsyncRuntime {
	/// Global sequential ID to verify task ID's are used on correct runtimes
	uint32_t id;

	AsyncRuntimeOptions options;

	/// All unfinished tasks
	Vector(AsyncTask) tasks;

	/// Tasks waiting to be executed
	Vector(AsyncTaskId) ready_queue;

	/// Currently running task, `ASYNC_NO_CURRENT_TASK` if none
	AsyncTaskId current_task;

	/// Next local task ID to be assigned
	uint32_t next_task_id;

	/// CPU state before switching to the first task
	CpuState start_state;
} AsyncRuntime;

AsyncTask* async_find_task(AsyncRuntime* runtime, AsyncTaskId id);

void async_switch_next(AsyncRuntime* runtime, bool store_current_state);

[[maybe_unused]] static uint32_t runtime_id(AsyncTaskId id)
{
	assert(id != ASYNC_NO_CURRENT_TASK);
	return id >> 32;
}

[[maybe_unused]] static AsyncTaskId task_id(uint32_t runtime_id, LocalAsyncTaskId local_task_id)
{
	return (uint64_t)runtime_id << 32 | local_task_id;
}

#endif
