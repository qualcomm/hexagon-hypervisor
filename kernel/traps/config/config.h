/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_CONFIG_H
#define H2K_CONFIG_H 1

#include <c_std.h>
#include <context.h>
#include <vm.h>
#include <vmint.h>

#define UNIT sizeof(u32_t)
#define ROUND(expr) ((((expr) + UNIT - 1) / UNIT) * UNIT)

#define BITS_PER_WORD 32
#define BYTES_PER_WORD (sizeof(u32_t) / sizeof(u8_t))
#define PHYS_PER_WORD (sizeof(u32_t) / sizeof(physint_t))

#define VMBLOCK_SPACE ROUND(sizeof(H2K_vmblock_t))

// pending
#define PENDING_WORDS(ints) ((ints + BITS_PER_WORD - 1) / BITS_PER_WORD)
#define PENDING_SPACE(ints) ROUND(PENDING_WORDS(ints) * BYTES_PER_WORD)

// enable
#define ENABLE_WORDS(ints) ((ints + BITS_PER_WORD - 1) / BITS_PER_WORD)
#define ENABLE_SPACE(ints) ROUND(ENABLE_WORDS(ints) * BYTES_PER_WORD)

// percpu_mask
#define MASKPTR_SPACE(cpus, ints) (ints > 0 ? ROUND((cpus * sizeof(bitmask_t *))) : 0)
#define MASK_WORDS_PERCPU(ints) ((ints + BITS_PER_WORD - 1) / BITS_PER_WORD)
#define MASK_WORDS(cpus, ints) (cpus * MASK_WORDS_PERCPU(ints))
#define MASK_SPACE(cpus, ints) ROUND(MASK_WORDS(cpus, ints) * BYTES_PER_WORD)

// int_v2p
#define PHYSINT_WORDS(ints) ((ints + PHYS_PER_WORD - 1) / PHYS_PER_WORD)
#define PHYSINT_SPACE(ints) ROUND(PHYSINT_WORDS(ints) * BYTES_PER_WORD)

/* Interrupt Handler Info */
#define INTINFO_SPACE(ints) (((ints > 0) ? 3 : 2) * sizeof(H2K_vm_int_opinfo_t))

// cpu_contexts
#define CONTEXT_SPACE(cpus) ROUND(cpus * sizeof(H2K_thread_context))

#define VMBLOCK_SIZE(cpus, ints) \
	(VMBLOCK_SPACE + \
	 PENDING_SPACE(ints) + \
	 ENABLE_SPACE(ints) + \
	 MASKPTR_SPACE(cpus, ints) + \
	 MASK_SPACE(cpus, ints) + \
	 PHYSINT_SPACE(ints) + \
	 CONTEXT_SPACE(cpus) + \
	 INTINFO_SPACE(ints) + \
	 (H2K_VMBLOCK_ALIGN - 1))
   // space to align if needed

typedef enum {
	CONFIG_SETFATAL,
	CONFIG_VMBLOCK_SIZE,
	CONFIG_VMBLOCK_INIT,
	CONFIG_MAX
} config_type_t;

typedef enum {
	SET_STORAGE,
	SET_PMAP_TYPE,
	SET_PRIO_TRAPMASK,
	SET_CPUS_INTS,
	MAP_PHYS_INTR,
	NUM_OPS
} vmblock_init_op_t;

u32_t H2K_trap_config(u32_t configtype, void *ptr, u32_t val2, u32_t val3, u32_t val4, H2K_thread_context *me) IN_SECTION(".text.config.config");
u32_t H2K_trap_config_setfatal(u32_t unused, void *handler, u32_t unused2, u32_t unused3, u32_t unused4, H2K_thread_context *me) IN_SECTION(".text.config.config");
u32_t H2K_trap_config_vmblock_size(u32_t unused, void *unused2, u32_t num_cpus, u32_t num_ints, u32_t unused3, H2K_thread_context *me) IN_SECTION(".text.config.config");
u32_t H2K_trap_config_vmblock_init(u32_t unused, void *ptr, u32_t op, u32_t arg1, u32_t arg2, H2K_thread_context *me) IN_SECTION(".text.config.config");

#endif

