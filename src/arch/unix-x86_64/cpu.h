#if !defined (INCL_ARCH_UNIX_X86_64_CPU_H)
#define INCL_ARCH_UNIX_X86_64_CPU_H

#include <stdint.h>
#include <stddef.h>

// Warning: do not modify the struct / rearrange fields
// without updating the corresponding assembly in task_switching.c

/// Arch-specific CPU state
typedef struct CpuState {
	uint64_t rbx;
	uint64_t rsp;
	uint64_t rbp;
	uint64_t r12;
	uint64_t r13;
	uint64_t r14;
	uint64_t r15;
} CpuState;

static_assert(offsetof(CpuState, rbx) == 0);
static_assert(offsetof(CpuState, rsp) == 8);
static_assert(offsetof(CpuState, rbp) == 16);
static_assert(offsetof(CpuState, r12) == 24);
static_assert(offsetof(CpuState, r13) == 32);
static_assert(offsetof(CpuState, r14) == 40);
static_assert(offsetof(CpuState, r15) == 48);

#endif
