/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_H
#define BLAST_H 1

#include <h2.h>

#define RENAME_PREFIX blast
#include "rename.h"
#undef RENAME_PREFIX

static inline int blast_thread_create(void *pc, void *stack, void *arg, 
	unsigned int prio, unsigned int asid, unsigned int hw_bitmask)
{
	/*
	 * H2's thread create prototype is:
	 * H2K_thread_create(u32_t pc, u32_t sp, u32_t arg, u32_t prio, u32_t trapmask,
	 *    H2K_thread_context *me)
	 * The trap automatically fills in the *me pointer, so it's really just that
	 * ASID thing that needs changing.  Just drop that on the floor and fill trapmask.
	 * drop the hw_bitmask on the floor too.
	 */
	//  might as well fudge the priority since it's spread all over.

	return h2_thread_create(pc,stack,arg,prio/8,0xffffffff);
}

static inline void blast_thread_exit(int status)
{
	//  old blast was calling TLS free routines and calling blast_thread_stop_asm(status)
	h2_thread_stop();
}

static inline void blast_mutex_destroy(h2_mutex_t *lock)
{
	//  supposed to clean up any kernel-associated memory with mutexes...
	return;
}

static inline void blast_thread_join(int threadid, int *status)
{
}

static inline unsigned short blast_sem_get_val(h2_sem_t *sem)
{
	//  This doesn't even seem safe, but that's how they did it in blast
	return sem->val;  
}

static inline void blast_sem_destroy(h2_sem_t *sem)
{
	//  looks like there was some queue related stuff...
	return;
}

#endif

