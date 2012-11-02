/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_vm.h
 * 
 * Boot a VM
 */

#ifndef H2_VM_H
#define H2_VM_H 1

#ifndef H2K_VMOP_H
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

int h2_vmop_trap(vmop_t op, unsigned int arg1, unsigned int arg2, unsigned int arg3, unsigned int arg4, unsigned int arg5);

/* basically thread_create but with a vmblock */
static inline int h2_vmboot(void *pc, void *stack, unsigned int arg, unsigned int prio, unsigned int vm) {

	return h2_vmop_trap(VMOP_BOOT, (unsigned int)pc, (unsigned int)stack, arg, prio, vm);
}

static inline int h2_vmstatus(vmop_status_t op, unsigned int vm) {

	return h2_vmop_trap(VMOP_STATUS, op, vm, 0, 0, 0);
}
	

static inline int h2_vmfree(unsigned int vm) {

	return h2_vmop_trap(VMOP_FREE, vm, 0, 0, 0, 0);
}

#endif

