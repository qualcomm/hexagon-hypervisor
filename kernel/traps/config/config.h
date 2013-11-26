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
#include <max.h>
#include <h2_common_config.h>

#define UNIT sizeof(u32_t)
#define ROUND(expr) ((((expr) + UNIT - 1) / UNIT) * UNIT)

#define BITS_PER_WORD 32
#define BYTES_PER_WORD (sizeof(u32_t) / sizeof(u8_t))
#define PHYS_PER_WORD (sizeof(u32_t) / sizeof(physint_t))

#define VMBLOCK_SPACE (sizeof(H2K_vmblock_t))

// cpu_contexts
#define CONTEXT_SPACE(cpus) ROUND(cpus * sizeof(H2K_thread_context))

#ifdef DO_EXT_SWITCH
// extended contexts
#define EXT_CONTEXT_SPACE(cpus) ROUND(cpus * sizeof(H2K_ext_context))
#endif

// interrupt handler info
#define INTINFO_SPACE(ints) (((ints > 0) ? 3 : 2) * sizeof(H2K_vm_int_opinfo_t))

// percpu_mask
#define MASKPTR_SPACE(cpus, ints) (ints > 0 ? ROUND((cpus * sizeof(bitmask_t *))) : 0)
#define MASK_WORDS_PERCPU(ints) ((ints + BITS_PER_WORD - 1) / BITS_PER_WORD)
#define MASK_WORDS(cpus, ints) (cpus * MASK_WORDS_PERCPU(ints))
#define MASK_SPACE(cpus, ints) ROUND(MASK_WORDS(cpus, ints) * BYTES_PER_WORD)

// pending
#define PENDING_WORDS(ints) ((ints + BITS_PER_WORD - 1) / BITS_PER_WORD)
#define PENDING_SPACE(ints) ROUND(PENDING_WORDS(ints) * BYTES_PER_WORD)

// enable
#define ENABLE_WORDS(ints) ((ints + BITS_PER_WORD - 1) / BITS_PER_WORD)
#define ENABLE_SPACE(ints) ROUND(ENABLE_WORDS(ints) * BYTES_PER_WORD)

// int_v2p
#define PHYSINT_WORDS(ints) ((ints + PHYS_PER_WORD - 1) / PHYS_PER_WORD)
#define PHYSINT_SPACE(ints) ROUND(PHYSINT_WORDS(ints) * BYTES_PER_WORD)

#define VMBLOCK_SIZE_BASE(cpus, ints)						\
	(VMBLOCK_SPACE +															\
	 CONTEXT_SPACE(cpus) +												\
	 INTINFO_SPACE(ints) +												\
	 MASKPTR_SPACE(cpus, ints) +									\
	 MASK_SPACE(cpus, ints) +											\
	 PENDING_SPACE(ints) +												\
	 ENABLE_SPACE(ints) +													\
	 PHYSINT_SPACE(ints + PERCPU_INTERRUPTS))

#ifdef DO_EXT_SWITCH
#define VMBLOCK_SIZE(cpus, ints, ext)			 \
	(VMBLOCK_SIZE_BASE(cpus, ints) +				 \
	 (ext ? (EXT_CONTEXT_SPACE(cpus)) : 0) + \
	 (H2K_VMBLOCK_ALIGN - 1))
   // space to align if needed
#else
#define VMBLOCK_SIZE(cpus, ints)								\
	(VMBLOCK_SIZE_BASE(cpus, ints) +							\
	 (H2K_VMBLOCK_ALIGN - 1))
   // space to align if needed
#endif

u32_t H2K_trap_config(config_type_t configtype, u32_t val1, vmblock_init_op_t val2, u32_t val3, u32_t val4, H2K_thread_context *me) IN_SECTION(".text.config.config");
u32_t H2K_trap_config_vmblock_init(u32_t unused, u32_t vm, vmblock_init_op_t op, u32_t arg1, u32_t arg2, H2K_thread_context *me) IN_SECTION(".text.config.config");
u32_t H2K_trap_config_stlb_alloc(u32_t unused, u32_t sets, vmblock_init_op_t unused2, u32_t unused3, u32_t unused4, H2K_thread_context *me);

#endif

