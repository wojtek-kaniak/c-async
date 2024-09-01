#if !defined (INCL_COMMON_H)
#define INCL_COMMON_H

#include "async.h"

[[noreturn]] void async_alloc_fail(AsyncAllocOptions opts);

[[noreturn]] void async_panic(const char* fmt, ...);

void async_log(const char* fmt, ...);

#endif
