#include <assert.h>
#include <stddef.h>
#include <string.h>

#include "async.h"
#include "common.h"
#include "generic.h"

// Uncomment when editing in an IDE:
#if !defined (TYPE)
#define TYPE int
#endif

#if !defined Vector
#define Vector(type) CONCAT2(Vector_, type)
#endif

#define VECTOR_NAME CONCAT2(Vector_, TYPE)
#define MIN_CAPACITY 4

typedef struct VECTOR_NAME {
	TYPE* items;
	size_t capacity;
	size_t length;
} VECTOR_NAME;

[[maybe_unused]] static VECTOR_NAME CONCAT2(VECTOR_NAME, _create)(AsyncAllocOptions alloc, size_t capacity)
{
	if (capacity < MIN_CAPACITY)
		capacity = MIN_CAPACITY;

	TYPE* items = alloc.malloc(capacity * sizeof(TYPE));

	if (items == nullptr)
		async_alloc_fail(alloc);

	return (VECTOR_NAME) {
		.items = items,
		.capacity = capacity,
		.length = 0,
	};
}

[[maybe_unused]] static void CONCAT2(VECTOR_NAME, _set_length)
	(VECTOR_NAME* self, AsyncAllocOptions alloc, size_t length)
{
	if (length > self->capacity)
	{
		size_t new_capacity = self->capacity;
		while (length > new_capacity)
			new_capacity *= 2;

		self->items = alloc.realloc(self->items, new_capacity * sizeof(TYPE));

		if (self->items == nullptr)
			async_alloc_fail(alloc);
	}

	self->length = length;
}

[[maybe_unused]] static void CONCAT2(VECTOR_NAME, _add)(VECTOR_NAME* self, AsyncAllocOptions alloc, TYPE value)
{
	CONCAT2(VECTOR_NAME, _set_length)(self, alloc, self->length + 1);
	self->items[self->length - 1] = value;
}

[[maybe_unused]] static TYPE CONCAT2(VECTOR_NAME, _delete)(VECTOR_NAME* self, size_t ix)
{
	assert(ix < self->length);

	TYPE item = self->items[ix];

	memmove(self->items + ix, self->items + ix + 1, (self->length - ix - 1) * sizeof(TYPE));

	self->length = self->length - 1;

	return item;
}

[[maybe_unused]] static void CONCAT2(VECTOR_NAME, _free)(VECTOR_NAME* self, AsyncAllocOptions alloc)
{
	alloc.free(self->items);
	self->length = 0;
	self->capacity = 0;
}
