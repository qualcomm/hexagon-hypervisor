/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/* Don't do #ifndef thing */

#include <h2.h>

#ifndef RENAME_PREFIX
#error define RENAME_PREFIX before inclusion
#endif

#define RENAME_PASTE3_REAL(X,Y,Z) X##Y##Z
#define RENAME_PASTE3(X,Y,Z) RENAME_PASTE3_REAL(X,Y,Z)
#define RENAME_PASTE2_REAL(X,Y) X##Y
#define RENAME_PASTE2(X,Y) RENAME_PASTE2_REAL(X,Y)

#define REDEF_FUNCTION(RET,NAME,ARGS,CALLARGS) static inline RET RENAME_PASTE3(RENAME_PREFIX,_,NAME) ARGS { return RENAME_PASTE2(h2_,NAME) CALLARGS ; }
#define RETYPEDEF(NAME) typedef RENAME_PASTE2(h2_,NAME) RENAME_PASTE3(RENAME_PREFIX,_,NAME);

/*  "blast_alloc.h"  */

REDEF_FUNCTION(void *,malloc,(unsigned int size),(size));
REDEF_FUNCTION(void *,calloc,(unsigned int elsize, unsigned int num),(elsize,num));
REDEF_FUNCTION(void *,realloc,(void *ptr, int newsize),(ptr,newsize));
REDEF_FUNCTION(void ,free,(void *ptr),(ptr));

/*  "blast_allsignal.h"  */

RETYPEDEF(allsignal_t)

REDEF_FUNCTION(void ,allsignal_init,(h2_allsignal_t *signal),(signal));
REDEF_FUNCTION(void ,allsignal_wait,(h2_allsignal_t *signal, unsigned int mask),(signal,mask));
REDEF_FUNCTION(void ,allsignal_signal,(h2_allsignal_t *signal, unsigned int mask),(signal,mask));

/*  "blast_anysignal.h"  */

RETYPEDEF(anysignal_t)

REDEF_FUNCTION(void ,anysignal_init,(h2_anysignal_t *signal),(signal));
REDEF_FUNCTION(unsigned int ,anysignal_wait,(h2_anysignal_t *signal, unsigned int mask),(signal,mask));
REDEF_FUNCTION(unsigned int ,anysignal_set,(h2_anysignal_t *signal, unsigned int mask),(signal,mask));
REDEF_FUNCTION(unsigned int ,anysignal_get,(h2_anysignal_t *signal),(signal));
REDEF_FUNCTION(unsigned int ,anysignal_clear,(h2_anysignal_t *signal, unsigned int mask),(signal,mask));

/*  "blast_barrier.h"  */

RETYPEDEF(barrier_t)

#define H2_BARRIER_SERIAL_THREAD 1
#define H2_BARRIER_OTHER 0

REDEF_FUNCTION(int ,barrier_init,(h2_barrier_t *barrier, unsigned int threads_total),(barrier,threads_total));
REDEF_FUNCTION(int ,barrier_wait,(h2_barrier_t *barrier),(barrier));

/*  "blast_cond.h"  */

//typedef h2_cond_t blast_cond_t;
RETYPEDEF(cond_t)

REDEF_FUNCTION(void ,cond_signal,(h2_cond_t *cond),(cond));
REDEF_FUNCTION(void ,cond_broadcast,(h2_cond_t *cond),(cond));

// move to blast.h; need wrapper because blast_mutex == h2_rmutex
/* REDEF_FUNCTION(void ,cond_wait,(h2_cond_t *cond,h2_mutex_t *mutex),(cond,mutex)); */

REDEF_FUNCTION(void ,cond_init,(h2_cond_t *cond),(cond));

/*  "blast_config.h"  */

REDEF_FUNCTION(unsigned int ,config_add_thread_storage,(void *buf, unsigned int size),(buf,size));

/*  "blast_cycles.h"  */

REDEF_FUNCTION(unsigned long long int ,get_pcycles,(void),());
REDEF_FUNCTION(unsigned long long int ,get_tcycles,(void),());
REDEF_FUNCTION(unsigned long long int ,get_core_pcycles,(void),());

#if 0
REDEF_FUNCTION(void, profile_enable,(int enable),(enable));
REDEF_FUNCTION(void, profile_reset_idle_pcycles,(void),());
REDEF_FUNCTION(void, profile_reset_thread_pcycles,(int thread_id),(thread_id));
REDEF_FUNCTION(void, profile_get_idle_pcycles,(unsigned long long *pcycles),(pcycles));
REDEF_FUNCTION(void, profile_get_thread_pcycles,(int thread_id, unsigned long long *pcycles),(thread_id, pcycles));
#endif

/*  "blast_fastint.h"  */

// REDEF_FUNCTION(void ,register_fastint,(int intno, int (*fn)(int)),(intno,fn));
// REDEF_FUNCTION(void ,deregister_fastint,(int intno),(intno));

/*  "blast_futex.h"  */

REDEF_FUNCTION(int ,futex_wait,(void *lock, int val),(lock,val));
REDEF_FUNCTION(int ,futex_wake,(void *lock, int n_to_wake),(lock,n_to_wake));

/*  "blast_intwait.h"  */

REDEF_FUNCTION(int ,intwait,(unsigned long long int mask),(mask));

/*  "blast_libkernel.h  */

#define H2_PREINIT_L2CACHE_SIZE_0K 0
#define H2_PREINIT_L2CACHE_SIZE_64K 1
#define H2_PREINIT_L2CACHE_SIZE_128K 2
#define H2_PREINIT_L2CACHE_SIZE_256K 3

REDEF_FUNCTION(void ,init,(unsigned long long int *memmap),(memmap));
//REDEF_FUNCTION(void ,preinit_hthread_startup,(unsigned int mask),(mask));
//REDEF_FUNCTION(void ,preinit_l2cache_size,(int size),(size));
//REDEF_FUNCTION(void ,preinit_relocate_tcm,(void *addr),(addr));

/*  "blast_mutex.h"  */

//RETYPEDEF(mutex_t)
/* typedef h2_mutex_t blast_mutex_t; */
typedef h2_rmutex_t blast_mutex_t;

// move to blast.h; need wrapper because blast_mutex == h2_rmutex
/* REDEF_FUNCTION(void ,mutex_lock,(h2_mutex_t *lock),(lock));		/\* blocking *\/ */
/* REDEF_FUNCTION(void ,mutex_unlock,(h2_mutex_t *lock),(lock));	/\* unlock *\/ */
/* REDEF_FUNCTION(int ,mutex_trylock,(h2_mutex_t *lock),(lock));	/\* just try... 1 if successful, 0 if not *\/ */
/* REDEF_FUNCTION(void ,mutex_init,(h2_mutex_t *lock),(lock)) 	/\* initialize it... *\/ */

/*  "blast_pipe.h"  */

RETYPEDEF(pipe_data_t) //typedef h2_pipe_data_t blast_pipe_data_t;
RETYPEDEF(pipe_t) // typedef h2_pipe_t blast_pipe_t;

REDEF_FUNCTION(h2_pipe_t *,pipe_alloc,(unsigned int size_in_bytes),(size_in_bytes));
REDEF_FUNCTION(h2_pipe_t *,pipe_create,(h2_pipe_t *pipe, h2_pipe_data_t *data, int data_elements),(pipe,data,data_elements));
REDEF_FUNCTION(void ,pipe_free,(h2_pipe_t *pipe),(pipe));

REDEF_FUNCTION(void ,pipe_send,(h2_pipe_t *pipe, h2_pipe_data_t data),(pipe,data));
REDEF_FUNCTION(h2_pipe_data_t , pipe_recv,(h2_pipe_t *pipe),(pipe));

REDEF_FUNCTION(int ,pipe_trysend,(h2_pipe_t *pipe, h2_pipe_data_t data),(pipe,data));
REDEF_FUNCTION(h2_pipe_data_t , pipe_tryrecv,(h2_pipe_t *pipe, int *success),(pipe,success));

/*  "blast_prefetch.h"  */

REDEF_FUNCTION(void ,set_prefetch,(unsigned int settings),(settings));

/*  "blast_printf.h"  */

#define blast_printf(...) h2_printf(__VA_ARGS__)

/*  "blast_prio.h"  */

// Check the names on these
//REDEF_FUNCTION(int ,get_prio,(void),());
//REDEF_FUNCTION(int ,set_prio,(unsigned int threadid, unsigned int newprio),(threadid,newprio));

/*  "blast_rmutex.h"  */

//  Other macros seem to mess this up.
//RETYPEDEF(rmutex_t) // typedef h2_rmutex_t blast_rmutex_t;
typedef h2_rmutex_t blast_rmutex_t;

REDEF_FUNCTION(void ,rmutex_lock,(h2_rmutex_t *lock),(lock));
REDEF_FUNCTION(void ,rmutex_unlock,(h2_rmutex_t *lock),(lock));
REDEF_FUNCTION(int ,rmutex_trylock,(h2_rmutex_t *lock),(lock));
REDEF_FUNCTION(void ,rmutex_init,(h2_rmutex_t *lock),(lock));

/*  "blast_sem.h"  */
//RETYPEDEF(sem_t) // typedef h2_sem_t blast_sem_t;
typedef h2_sem_t blast_sem_t;

REDEF_FUNCTION(int ,sem_add,(h2_sem_t *sem, unsigned int amt),(sem,amt));
REDEF_FUNCTION(int ,sem_up,(h2_sem_t *sem),(sem));
REDEF_FUNCTION(int ,sem_down,(h2_sem_t *sem),(sem));
REDEF_FUNCTION(int ,sem_trydown,(h2_sem_t *sem),(sem));
REDEF_FUNCTION(void ,sem_init,(h2_sem_t *sem),(sem));
REDEF_FUNCTION(void ,sem_init_val,(h2_sem_t *sem,unsigned short val),(sem,val));

/*  "blast_thread.h"  */

//REDEF_FUNCTION(int ,thread_create,(void *pc, void *stack, void *arg, unsigned int prio, unsigned int asid),(pc,stack,arg,prio,asid));
REDEF_FUNCTION(void ,thread_stop,(void),());
REDEF_FUNCTION(int ,thread_myid,(void),());
REDEF_FUNCTION(void ,yield,(void),());
REDEF_FUNCTION(void ,thread_set_tid,(unsigned int tid),(tid));
REDEF_FUNCTION(int ,thread_get_tid,(void),());

