/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_CONFIG_H
#define H2_COMMON_CONFIG_H 1

typedef enum {
	SET_PMAP_TYPE,
	SET_FENCES,
	SET_PRIO_TRAPMASK,
	SET_CPUS_INTS,
	MAP_PHYS_INTR,
	NUM_OPS
} vmblock_init_op_t;

#define MAP_PHYS_INTR_CPU_BITS 16
#define CONFIG_PHYSINT_CPUID(phys, cpu) \
	(((phys) << MAP_PHYS_INTR_CPU_BITS) | ((cpu) & ((0x1 << MAP_PHYS_INTR_CPU_BITS) - 1)))

#endif
