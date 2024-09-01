// The only header file defining the public API
#if !defined(INCL_ASYNC_H)
#define INCL_ASYNC_H

#include <stddef.h>
#include <stdint.h>

/// Globally unique task ID
typedef uint64_t AsyncTaskId;

typedef struct AsyncAllocOptions {
	/// alloc function with stdlib `malloc` semantics
	void* (*malloc)(size_t);

	/// realloc function corresponding to `malloc`
	void* (*realloc)(void*, size_t);

	/// free function corresponding to `malloc`
	void (*free)(void*);

	/// Called when malloc/realloc returns null
	void (*async_alloc_fail)();
} AsyncAllocOptions;

typedef struct AsyncRuntimeOptions {
	/// Default minimal stack size in bytes
	size_t stack_size;

	/// Memory alloc functions to be used to allocate space for internal runtime structures and task stacks
	AsyncAllocOptions alloc_options;
} AsyncRuntimeOptions;

extern AsyncRuntimeOptions ASYNC_DEFAULT_RUNTIME_OPTS;

/// Initialize an async runtime on the current thread with default options
void async_runtime_create();

/// See `async_runtime_create`
void async_runtime_create_with_opts(AsyncRuntimeOptions options);

/// Start an async runtime on the current thread
/// Returns when no tasks left are left
void async_runtime_start();

/// Free all resources owned by the current thread's runtime
/// Panics if called when the runtime has any tasks remaining
void async_runtime_free();

/// Bit flags to be OR'ed together
typedef enum AsyncTaskFlags : uint32_t {
	/// `async_runtime_start` will return if only these tasks are left
	ASYNC_TASK_BACKGROUND = 1,
} AsyncTaskFlags;

typedef struct AsyncTaskOptions {
	/// Flags OR'ed together
	AsyncTaskFlags flags;

	/// Stack size in bytes, 0 to use runtime's default stack size
	size_t stack_size;
} AsyncTaskOptions;

extern AsyncTaskOptions ASYNC_DEFAULT_TASK_OPTS;

/// `data` - optional state passed to the `task` function
AsyncTaskId async_spawn(void (*task)(uintptr_t), uintptr_t data);

/// See `async_spawn`
AsyncTaskId async_spawn_with_opts(void (*task)(uintptr_t), uintptr_t data, AsyncTaskOptions options);

/// Yield to other tasks
void async_yield();

/// Get the identifier of the currently running task
AsyncTaskId async_current_task();

#endif
