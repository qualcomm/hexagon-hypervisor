/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLAST_H
#define BLAST_H 1

#include <h2.h>

#define RENAME_PREFIX blast
#include <h2_rename.h>
#undef RENAME_PREFIX

#include <blast_fd.h>
#include <blast_tls.h>
#include <blast_power.h>

#include <stddef.h>
#include <assert.h>

//  Signal used to indicate to anybody waiting for an interrupt that
//  it was deregistered.

#define SIGNAL_INT_ABORT 31
#define SIG_INT_ABORT (1<<SIGNAL_INT_ABORT)

#define EOK 0

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

static inline void blast_rmutex_destroy(blast_rmutex_t *lock)
{
	return;
}

// because blast_mutex_t is used for both mutex and rmutex
// blast_mutex_t is typdeffed to h2_rmutex_t
static inline void blast_mutex_lock(blast_mutex_t *lock)
{
	h2_mutex_lock(&lock->mutex);
}

static inline void blast_mutex_unlock(blast_mutex_t *lock)
{
	h2_mutex_unlock(&lock->mutex);
}

static inline int blast_mutex_trylock(blast_mutex_t *lock)
{
	return h2_mutex_trylock(&lock->mutex);
}

static inline void blast_mutex_init(blast_mutex_t *lock)
{
	h2_mutex_init(&lock->mutex);
}
static inline void blast_mutex_destroy(blast_mutex_t *lock)
{
	return;
}

// fake up blast_pimutex as h2_rmutex
static inline void blast_pimutex_init(blast_mutex_t *lock)
{
	h2_rmutex_init(lock);
}

static inline void blast_pimutex_destroy(blast_mutex_t *lock)
{
	return;
}

static inline void blast_pimutex_lock(blast_mutex_t *lock)
{
	h2_rmutex_lock(lock);
}

static inline void blast_pimutex_unlock(blast_mutex_t *lock)
{
	h2_rmutex_unlock(lock);
}

static inline int blast_pimutex_trylock(blast_mutex_t *lock)
{
	return h2_rmutex_trylock(lock);
}

int blast_thread_join(int threadid, int *status);

static inline unsigned short blast_sem_get_val(volatile h2_sem_t *sem)
{
	return sem->val;  
}

static inline void blast_sem_destroy(volatile h2_sem_t *sem)
{
	return;
}

// because blast_mutex_t is used for both mutex and rmutex
// blast_mutex_t is typdeffed to h2_rmutex_t
static inline void blast_cond_wait(h2_cond_t *cond, h2_rmutex_t *mutex)
{
	h2_cond_wait_rmutex(cond, mutex);
}

static inline void blast_cond_destroy(h2_cond_t *cond) 
{
	return;
}

static inline void blast_barrier_destroy(h2_barrier_t *barrier)
{
	return;
}

static inline void blast_thread_set_name(unsigned long long name0, 
	unsigned long long name1)
{
	return;
}

int blast_thread_create(void *pc, void *stack, void *arg, 
												unsigned int prio, unsigned int asid, unsigned int hw_bitmask);

void blast_thread_exit(int status);

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

static inline void blast_register_fastint(int intno, int (*fn)(int))
{
	h2_register_fastint(32+intno,fn);
}

static inline void blast_deregister_fastint(int intno)
{
	h2_deregister_fastint(32+intno);
}

static inline void blast_exit(int status)
{
	exit(status);
}

/* from blast_utcp.h */
#define blast_get_my_utcb(pUgp)        __asm__ __volatile__ ( " %0 = ugp " :"=r"(pUgp) ) ; 

static inline void blast_anysignal_destroy(blast_anysignal_t *signal)
{
	return;
}

unsigned int blast_get_my_anysignal();

//void blast_deregister_fastint(int intno);

//  Not really a part of the BLAST API, but it needs to happen somewhere.
void l2_controller_init(void);

// Thread configs are ignored now, but the need to be defined for sources:
#define BLAST_THREAD_CFG_BITMASK_HT0      0x00000001
#define BLAST_THREAD_CFG_BITMASK_HT1      0x00000002
#define BLAST_THREAD_CFG_BITMASK_HT2      0x00000004
#define BLAST_THREAD_CFG_BITMASK_HT3      0x00000008
#define BLAST_THREAD_CFG_BITMASK_HT4      0x00000010
#define BLAST_THREAD_CFG_BITMASK_HT5      0x00000020
#define BLAST_THREAD_CFG_BITMASK_ALL      0x000000ff

#define BLAST_THREAD_CFG_USE_RAM          0x00000000
#define BLAST_THREAD_CFG_USE_TCM          0x00000100

#define blast_thread_wait_for_idle  blast_power_wait_for_idle
#define blast_thread_wait_for_active blast_power_wait_for_active

#ifdef __cplusplus
}
#endif //__cplusplus

#endif

