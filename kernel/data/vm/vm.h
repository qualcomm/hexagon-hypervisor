/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VM_H
#define H2K_VM_H 1

/* How does this work?  */

/* Trigger interrupt: 
 * * set in pending
 * * look up cpu (TBD: array, or check first enabled?)
 * * look up context
 * * mark thread w/ interrupt vmwork
 * * If ready, try to int/wakeup dest
 */

/* Get interrupt:
 * * For each wordsize, ctz(pending & my_enables)
 * * Find lowest interrupt, clear from pending, my_enables
 */

/* Int Ack:
 * Set in my_enables 
 */

/* TBD:
 * Configure call?
 * Supporting BLAST-like schedulers?
 * Supporting priority?
 * Eliminate iteration?
 * Supporting multiple listeners?
 */

#include <c_std.h>
#include <vmdefs.h>

typedef u8_t physint_t;

typedef u32_t bitmask_t;

typedef struct H2K_vmblock_struct {
	u8_t num_cpus;
	u8_t pad;
	u16_t num_ints;
	/* Pending Virtual Interrupts for this VM */
	bitmask_t *pending;
	/* Global enable */
	bitmask_t *enable;
	/* For each cpu, enable masks (which cpu, enabled or masked, etc) 0==disabled */
	bitmask_t **percpu_mask;
	/* Mapping back to the HW interrupt (if applicable) */
	physint_t *int_v2p;
	/* Mapping of CPUs to Thread Contexts */
	struct _h2_thread_context **cpu_contexts;
	/* physical memory map, page table style */
	u32_t *pmap;
} H2K_vmblock_t;

u32_t H2K_vm_vmblock_size(u8_t num_cpus, u16_t num_ints) IN_SECTION(".text.misc.vmblock");
void  H2K_vm_vmblock_init(H2K_vmblock_t *vmblock, u8_t num_cpus, u16_t num_ints, u32_t *pmap) IN_SECTION(".text.init.vmblock");

#endif

