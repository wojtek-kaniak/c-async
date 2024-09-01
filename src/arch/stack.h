#if !defined (INCL_ARCH_STACK_H)
#define INCL_ARCH_STACK_H

#include <stddef.h>

typedef struct Stack {
	/// Address of the lowest allocated byte, including guard pages
	void* address;

	/// Full size, including guard pages
	size_t full_size;

	/// Stack size, excluding guard pages
	size_t size;

	/// Starting (high) address of the stack
	void* start_address;
} Stack;

/// `size` - size of the stack in bytes, excluding guard pages.
/// Note that the actual size (returned as `Stack.size`) may be bigger, to account for page size alignment.
Stack async_create_stack(size_t size);

/// `address` - `Stack.address`
void async_free_stack(void* address, size_t full_size);

#endif
