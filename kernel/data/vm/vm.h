/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VM_H
#define H2K_VM_H 1

#include<asid.h>
#include <c_std.h>
#include <vmdefs.h>
#include <context.h>
#include <spinlock.h>

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

#define H2K_VMBLOCK_ALIGN 32
#define H2K_VMBLOCK_V2P_INVALID 0x0

typedef u16_t physint_t;
typedef u32_t bitmask_t;

typedef struct H2K_vmblock_struct {
	u32_t max_cpus;
	u32_t num_cpus;
	u32_t bestprio; 	/* best allowed priority */
	u32_t vmidx;
	u32_t num_ints;
	u32_t trapmask;  	/* allowed traps */
	H2K_spinlock_t lock;
	/* Linked List of free threads in this VM */
	H2K_thread_context *free_threads;
	/* Pointer to thread context storage */
	H2K_thread_context *contexts;
	/* Pending Virtual Interrupts for this VM */
	bitmask_t *pending;
	/* Global enable */
	bitmask_t *enable;
	/* For each cpu, enable masks (which cpu, enabled or masked, etc) 0==disabled */
	bitmask_t **percpu_mask;
	/* Mapping back to the HW interrupt (if applicable) */
	physint_t *int_v2p;
	struct H2K_vm_int_opinfo_struct *intinfo;  
	/* physical memory map, page table style */
	u32_t pmap;
	translation_type pmap_type;
} __attribute__((aligned(32))) H2K_vmblock_t;

#endif

