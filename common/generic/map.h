#include "async.h"
#include "generic.h"

// Uncomment when editing in an IDE:
// #if !defined (KEY_TYPE) || !defined (VALUE_TYPE)
// [[maybe_unused]] static size_t int_hash(int value) { return value; }
// #define KEY_TYPE int
// #define VALUE_TYPE long
// #define HASH_FN int_hash
// #endif

#if !defined (Map)
#define Map(key, value) CONCAT4(Map_, key, _, value)
#endif

#define BUCKET_COUNT 53

#define MAP_NAME Map(KEY_TYPE, VALUE_TYPE)
#define KVP_NAME CONCAT4(KeyValuePair_, KEY_TYPE, _, VALUE_TYPE)

typedef struct KVP_NAME {
	KEY_TYPE key;
	VALUE_TYPE value;
} KVP_NAME;

#define TYPE KVP_NAME
#include "vector.h"
#undef TYPE

typedef struct MAP_NAME {
	Vector(KVP_NAME) buckets[BUCKET_COUNT];
} MAP_NAME;

[[maybe_unused]]
static MAP_NAME CONCAT3(MAP_NAME, _, create)(AsyncAllocOptions alloc_opts)
{
	MAP_NAME map;

	for (size_t i = 0; i < BUCKET_COUNT; i++)
		map.buckets[i] = CONCAT3(Vector(KVP_NAME), _, create)(alloc_opts, 0);

	return map;
}

/// Returns null if the key is not present
[[maybe_unused]]
static VALUE_TYPE* CONCAT3(MAP_NAME, _, get)(MAP_NAME* map, KEY_TYPE key)
{
	size_t hash = HASH_FN(key);
	size_t bucket_ix = hash % BUCKET_COUNT;

	Vector(KVP_NAME) kvps = map->buckets[bucket_ix];

	for (size_t kvp_ix = 0; kvp_ix < kvps.length; kvp_ix++)
	{
		KVP_NAME* kvp = &kvps.items[kvp_ix];
		if (kvp->key == key)
			return &kvp->value;
	}

	return nullptr;
}

/// Returns `true` if the key was already present
[[maybe_unused]] 
static bool CONCAT3(MAP_NAME, _, insert)(MAP_NAME* map, AsyncAllocOptions alloc_opts, KEY_TYPE key, VALUE_TYPE value)
{
	VALUE_TYPE* value_ptr = CONCAT3(MAP_NAME, _, get)(map, key);

	if (value_ptr != nullptr)
		*value_ptr = value;
	else
	{
		size_t hash = HASH_FN(key);
		size_t bucket_ix = hash % BUCKET_COUNT;

		CONCAT2(Vector(KVP_NAME), _add)(&map->buckets[bucket_ix], alloc_opts, (KVP_NAME){
			.key = key,
			.value = value,
		});
	}
}

#undef MAP_NAME
#undef KVP_NAME
