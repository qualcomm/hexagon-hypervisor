/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_CONFIG_H
#define H2_CONFIG_H 1

typedef enum {
	SET_STORAGE_IDENT,
	SET_PMAP_TYPE,
	SET_PRIO_TRAPMASK,
	SET_CPUS_INTS,
	MAP_PHYS_INTR,
	NUM_OPS
} vmblock_init_op_t;

unsigned int h2_config_add_thread_storage(void *buf, unsigned int size);

unsigned int h2_config_setfatal(void *handler);

unsigned int h2_config_vmblock_size(char num_cpus, short num_ints);

void *h2_config_vmblock_init(void *ptr, vmblock_init_op_t op, unsigned int arg1, unsigned int arg2);

#endif

