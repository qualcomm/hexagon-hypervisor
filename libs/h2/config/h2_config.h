/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_CONFIG_H
#define H2_CONFIG_H 1

#include <h2_common_vmblock.h>
#include <h2_common_config.h>

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

