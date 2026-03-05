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

#include <h2_common_vmblock.h>
#include <h2_common_config.h>
#include <h2_common_asid.h>

typedef struct {
	translation_type type;
	H2K_offset_t base;
} h2_guest_pmap_t;

/** 
Raw interface for the config trap.  Do not use.  Instead, use an interface function such
as h2_config_vmblock_init().
@param[in] whichtrap	Which hardware configuration trap to use
@param[in] ptr		Address argument for trap
@param[in] a		First argument
@param[in] b		Second argument
@param[in] c		Third argument
@returns unsigned value on success, 0 on error
@dependencies None
*/

int h2_config_trap(config_type_t whichtrap, unsigned int a, unsigned int b, unsigned int c, unsigned int d);

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

/**
Allocate and initialize STLB
@returns number of STLB entries; -1 on failure
*/
int h2_config_stlb_alloc(void);

int h2_config_fatal_hook(unsigned int funcaddr, unsigned int arg);

#ifdef CLUSTER_SCHED
int h2_config_cluster_sched(unsigned int enable);
#endif

// FIXME: hack for setting noc table addresses

static inline int h2_config_set_noc(unsigned int master, unsigned int slave) {
	return h2_config_trap(CONFIG_NOC, master, slave, 0, 0);
}

/** @} */

#endif
