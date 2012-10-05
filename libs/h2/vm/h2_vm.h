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

/* basically thread_create but with a vmblock */
int h2_vmboot(void *pc, void *stack, unsigned int arg, unsigned int prio, unsigned int vm);

unsigned int h2_vmstatus(unsigned int vm);

#endif

