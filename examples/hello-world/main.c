#include <stdint.h>
#include <stdio.h>
#include <assert.h>

#include <async.h>

void task_a(uintptr_t data);
void task_b(uintptr_t data);

int main()
{
	async_runtime_create();

	async_spawn(task_a, 0);
	async_spawn(task_b, 42);

	async_runtime_start();
	async_runtime_free();
}

void task_a(uintptr_t data)
{
	assert(data == 0);

	puts("task a - 1");

	async_yield();

	puts("task a - 2");
}

void task_b(uintptr_t data)
{
	assert(data == 42);

	puts("task b - 1");

	async_yield();

	puts("task b - 2");
}
