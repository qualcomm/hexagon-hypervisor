/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_vm_imp.ref.c
 * 
 * Boot a VM - Implementation
 */

#include "h2_vm.h"

/* basically thread_create but with a vmblock */
int h2_vmboot(void *pc, void *stack, unsigned int arg, unsigned int prio, unsigned int vm) {
	return h2_vmop_trap(VMOP_BOOT, (unsigned int)pc, (unsigned int)stack, arg, prio, vm);
}

int h2_vmstatus(vmop_status_t op, unsigned int vm) {
	return h2_vmop_trap(VMOP_STATUS, op, vm, 0, 0, 0);
}

int h2_vmfree(unsigned int vm) {
	return h2_vmop_trap(VMOP_FREE, vm, 0, 0, 0, 0);
}
