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
	VMOP_MAX
} vmop_t;

typedef enum {
	VMOP_STATUS_STATUS,
	VMOP_STATUS_CPUS,
	VMOP_STATUS_MAX
} vmop_status_t;

#endif
