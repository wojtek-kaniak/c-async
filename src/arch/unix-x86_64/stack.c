#if defined (__x86_64__) && defined (__unix__)

#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>

#include "common.h"
#include "arch/stack.h"

static void assert_os(const char* function_name, bool condition)
{
	if (!condition)
	{
		perror(function_name);
		async_panic("syscall failed");
	}
}

static size_t next_multiple_of(size_t value, size_t alignment)
{
	return (value + alignment - 1) / alignment * alignment;
}

static const size_t guard_pages_low = 1;
static const size_t guard_pages_high = 1;

Stack async_create_stack(size_t size)
{
	assert(size > 0);

	size_t page_size = sysconf(_SC_PAGE_SIZE);

	// align stack size to the page size
	size = next_multiple_of(size, page_size);

	size_t full_size = size + (guard_pages_low + guard_pages_high) * page_size;

	void* addr = mmap(nullptr, full_size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
	assert_os("mmap", addr != nullptr);

	assert_os("mprotect", mprotect(addr, guard_pages_low * page_size, PROT_NONE) == 0);

	void* high_guard_page_addr = addr + guard_pages_low * page_size + size;
	assert_os("mprotect", mprotect(high_guard_page_addr, guard_pages_high * page_size, PROT_NONE) == 0);

	void* start_addr = high_guard_page_addr - 0x100;

	return (Stack) {
		.address = addr,
		.full_size = full_size,
		.size = size,
		.start_address = start_addr,
	};
}

void async_free_stack(void* address, size_t full_size)
{
	assert_os("munmap", munmap(address, full_size) == 0);
}

#endif
