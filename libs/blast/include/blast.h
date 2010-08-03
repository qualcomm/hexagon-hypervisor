/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_H
#define BLAST_H 1

#include <h2.h>

#include <blast_fd.h>
#include <blast_tls.h>
#define RENAME_PREFIX blast
#include <h2_rename.h>
#undef RENAME_PREFIX

//  Signal used to indicate to anybody waiting for an interrupt that
//  it was deregistered.

#define SIGNAL_INT_ABORT 31
#define SIG_INT_ABORT (1<<SIGNAL_INT_ABORT)

#define EOK 0

static inline int blast_thread_create(void *pc, void *stack, void *arg, 
	unsigned int prio, unsigned int asid, unsigned int hw_bitmask)
{
	/*
	 * H2's thread create prototype is:
	 * H2K_thread_create(u32_t pc, u32_t sp, u32_t arg, u32_t prio, u32_t trapmask,
	 *    H2K_thread_context *me)
	 * The trap automatically fills in the *me pointer, so it's really just that
	 * ASID thing that needs changing.  Just drop that on the floor and fill trapmask.
	 * drop the hw_bitmask on the floor
	 * might as well fudge the priority too 
	 */

	return h2_thread_create(pc,stack,arg,prio/8,0xffffffff);
}

static inline void blast_thread_exit(int status)
{
	//  old blast was calling TLS free routines and calling blast_thread_stop_asm(status)
	h2_thread_stop();
}

static inline void blast_rmutex_destroy(blast_rmutex_t *lock)
{
	return;
}

static inline void blast_mutex_destroy(blast_mutex_t *lock)
{
	return;
}

static inline void blast_thread_join(int threadid, int *status)
{
}

static inline unsigned short blast_sem_get_val(h2_sem_t *sem)
{
	return sem->val;  
}

static inline void blast_sem_destroy(h2_sem_t *sem)
{
	return;
}

static inline void blast_cond_destroy(h2_cond_t *cond) 
{
	return;
}

static inline void blast_barrier_destroy(h2_barrier_t *barrier)
{
	return;
}

static inline void blast_anysignal_destroy(blast_anysignal_t *signal)
{
	return;
}

static inline void blast_thread_set_name(unsigned long long name0, 
	unsigned long long name1)
{
	return;
}

static inline int blast_prio_get(unsigned int threadid)
{
	return -1; 
}

static inline int blast_prio_set(unsigned int threadid,unsigned int newprio)
{
	return h2_set_prio(threadid,newprio);
}

int blast_register_interrupt(int int_num, 
	h2_anysignal_t *int_signal, int signal_mask);

unsigned int blast_deregister_interrupt(int int_num);

static inline int blast_ack_interrupt(int intno)
{
	return 0;
}

static inline void blast_exit(int status)
{
	exit(status);
}

void blast_deregister_fastint(int intno);

//  Not really a part of the BLAST API, but it needs to happen somewhere.
void l2_controller_init(void);

#endif

