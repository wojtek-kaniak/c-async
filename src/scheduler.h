#if !defined (INCL_SCHEDULER_H)
#define INCL_SCHEDULER_H

#include "async.h"
#include "tasks.h"

void async_scheduler_enqueue(AsyncRuntime* runtime, AsyncTaskId task);
AsyncTaskId async_scheduler_next(AsyncRuntime* runtime);

#endif
