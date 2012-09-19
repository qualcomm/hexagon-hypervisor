/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_CONFIG_H
#define H2_CONFIG_H 1

/** @file h2_config.h
 @brief Configuration of the Virtual Machine
*/
/** @addtogroup h2 
@{ */

/**
Operations that can be done during vmblock initialization
*/

/* FIXME: These duplicate kernel/config/config.h and kernel/data/vm.h.  Should come from a
	 shared header file */

#ifndef H2K_VM_H

typedef union {
	struct {
		unsigned int size:4;
		unsigned int cccc:4;
		unsigned int xwru:4;
		unsigned int pages:20;
	};
	unsigned int raw;
} H2K_offset_t;

typedef union {
	unsigned int raw;
	struct {
		unsigned int cpuidx:16;
		unsigned int physint:16;
	};
} H2K_physint_config_t;

#endif

#ifndef H2K_CONFIG_H
typedef enum {
	SET_PMAP_TYPE,		/**< Set the physical map (or offset) and type */
	SET_FENCES,  /**< Set fence addresses for offset mapping */
	SET_PRIO_TRAPMASK,	/**< Set the best priority and trap mask */
	SET_CPUS_INTS,		/**< Set the number of CPUs and Interrupts */
	MAP_PHYS_INTR,		/**< Set a Physical to Guest Interrupt Map */
	NUM_OPS			/**< Used to determine range of valid operations, not a valid op */
} vmblock_init_op_t;
#endif

#define H2_MAP_PHYS_INTR_CPU_BITS 16
#define H2_CONFIG_PHYSINT_CPUID(phys, cpu) \
	(((phys) << H2_MAP_PHYS_INTR_CPU_BITS) | ((cpu) & ((0x1 << H2_MAP_PHYS_INTR_CPU_BITS) - 1)))

unsigned int h2_config_setfatal(void *handler);

/**
Initialize fields for a vm block
@param[in] vm  VM number
@param[in] op		Desired operation on the VM block
@param[in] arg1		First argument, usage varies on operation type
@param[in] arg2		Second argument, usage varies on operation type
@returns VM number; 0 on failure
@dependencies None
*/

unsigned int h2_config_vmblock_init(unsigned int vm, vmblock_init_op_t op, unsigned int arg1, unsigned int arg2);

/** @} */

#endif

