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
typedef enum {
	SET_STORAGE_IDENT, 	/**< Set the storage */
	SET_PMAP_TYPE,		/**< Set the physical mapping type */
	SET_PRIO_TRAPMASK,	/**< Set the best priority and trap mask */
	SET_CPUS_INTS,		/**< Set the number of CPUs and Interrupts */
	MAP_PHYS_INTR,		/**< Set a Physical to Guest Interrupt Map */
	NUM_OPS			/**< Used to determine range of valid operations, not a valid op */
} vmblock_init_op_t;

/**
Add storage for thread contexts.
@param[in] buf		Address of block of memory for thread contexts
@param[in] size		Number of bytes available at the address
@returns The number of threads created in the memory, or negative value on failure.
@dependencies None
*/

int h2_config_add_thread_storage(void *buf, unsigned int size);

unsigned int h2_config_setfatal(void *handler);

/**
Get the size required for a vm block
@param[in] num_cpus	Number of CPUs desired to configure
@param[in] num_ints	Number of interrupts desired to configure
@returns The number bytes required to create a vm block with the above configuration.
@dependencies None
*/

unsigned int h2_config_vmblock_size(char num_cpus, short num_ints);

/**
Initialize fields for a vm block
@param[in] ptr		Pointer to the VM Block
@param[in] op		Desired operation on the VM block
@param[in] arg1		First argument, usage varies on operation type
@param[in] arg2		Second argument, usage varies on operation type
@returns A pointer to the vm block, or NULL on failure?
@dependencies None
*/

void *h2_config_vmblock_init(void *ptr, vmblock_init_op_t op, unsigned int arg1, unsigned int arg2);

/** @} */

#endif

