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

/* FIXME: These duplicate kernel/config/config.h.  Should come from a
	 shared header file */

/**
Operations that can be done during vmblock initialization
*/
typedef enum {
	SET_STORAGE, 	/**< Set the storage */
	SET_PMAP_TYPE,		/**< Set the physical mapping type */
	SET_PRIO_TRAPMASK,	/**< Set the best priority and trap mask */
	SET_CPUS_INTS,		/**< Set the number of CPUs and Interrupts */
	MAP_PHYS_INTR,		/**< Set a Physical to Guest Interrupt Map */
	NUM_OPS			/**< Used to determine range of valid operations, not a valid op */
} vmblock_init_op_t;

#define H2_MAP_PHYS_INTR_CPU_BITS 16
#define H2_CONFIG_PHYSINT_CPUID(phys, cpu) \
	(((phys) << H2_MAP_PHYS_INTR_CPU_BITS) | ((cpu) & ((0x1 << H2_MAP_PHYS_INTR_CPU_BITS) - 1)))

unsigned int h2_config_setfatal(void *handler);

/**
Get the size required for a vm block
@param[in] num_cpus	Number of CPUs desired to configure
@param[in] num_ints	Number of interrupts desired to configure
@returns The number bytes required to create a vm block with the above configuration.
@dependencies None
*/

unsigned int h2_config_vmblock_size(unsigned int num_cpus, unsigned int num_ints);

/**
Initialize fields for a vm block
@param[in] ptr		Pointer to the VM Block
@param[in] op		Desired operation on the VM block
@param[in] arg1		First argument, usage varies on operation type
@param[in] arg2		Second argument, usage varies on operation type
@returns A pointer to the vm block, or NULL on failure
@dependencies None
*/

void *h2_config_vmblock_init(void *ptr, vmblock_init_op_t op, unsigned int arg1, unsigned int arg2);

/** @} */

#endif

