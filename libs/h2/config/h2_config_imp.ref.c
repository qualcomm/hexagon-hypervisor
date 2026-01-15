/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_config_imp.ref.c
 * 
 * @brief Configuration of the Virtual Machine - Implementation
 */

#include "h2_config.h"

unsigned int h2_config_vmblock_init(unsigned int vm, vmblock_init_op_t op, unsigned int arg1, unsigned int arg2) {
	return (unsigned int)h2_config_trap(CONFIG_VMBLOCK_INIT, vm, op, arg1, arg2);
}

int h2_config_stlb_alloc() {
	return h2_config_trap(CONFIG_STLB_ALLOC, 0, 0, 0, 0);
}

int h2_config_fatal_hook(unsigned int funcaddr, unsigned int arg)
{
	return h2_config_trap(CONFIG_FATAL_HOOK,funcaddr,arg,0,0);
}

#ifdef CLUSTER_SCHED
int h2_config_cluster_sched(unsigned int enable) {

	return h2_config_trap(CONFIG_CLUSTER_SCHED, enable, 0, 0, 0);
}
#endif
