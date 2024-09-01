#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined (__unix__)
#include <signal.h>
#endif

#include "async.h"
#include "common.h"

/// Wrapper function to abort if opts.async_alloc_fail returns
[[noreturn]] void async_alloc_fail(AsyncAllocOptions opts)
{
	opts.async_alloc_fail();
	async_panic("allocation failure");
}

[[noreturn]] void async_panic(const char* fmt, ...)
{
	va_list vararg;
	vfprintf(stderr, fmt, vararg);

	#if defined (DEBUG) && defined (__unix__)
	// trigger a core dump
	raise(SIGABRT);
	#endif

	abort();
}

void async_log(const char* fmt, ...)
{
	#if defined (CASYNC_VERBOSE)
	fprintf(stderr, "[c-async] ");

	va_list vararg;
	vfprintf(stderr, fmt, vararg);

	fprintf(stderr, "\n");
	#else
	(void)fmt;
	#endif
}
