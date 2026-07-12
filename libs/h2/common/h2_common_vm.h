/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_VM_H
#define H2_COMMON_VM_H 1

typedef enum {
	VMOP_BOOT,
	VMOP_STATUS,
	VMOP_FREE,
	VMOP_KILL_THREAD,
	VMOP_KILL_VM,
	VMOP_MAX
} vmop_t;

typedef enum {
	VMOP_STATUS_STATUS,
	VMOP_STATUS_CPUS,
	VMOP_STATUS_MAX
} vmop_status_t;

# define VMOP_STATUS_VMIDX_SELF 0
# define VMOP_KILL_VM_SELF 0

#endif
