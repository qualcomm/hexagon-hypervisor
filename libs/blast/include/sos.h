/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef SOS_H
#define SOS_H 1
/* "sos_alloc.h" */

#include <blast.h>

#define REDEF_FUNCTION(RET,NAME,ARGS,CALLARGS) static inline RET sos_##NAME ARGS { return blast_##NAME CALLARGS ; }

REDEF_FUNCTION(void *,malloc,(unsigned int size),(size));
REDEF_FUNCTION(void *,calloc,(unsigned int elsize, unsigned int num),(elsize,num));
REDEF_FUNCTION(void *,realloc,(void *ptr, int newsize),(ptr,newsize));
REDEF_FUNCTION(void ,free,(void *ptr),(ptr));

// #include "sos_futex.h"

REDEF_FUNCTION(int ,futex_wait,(void *lock, int val),(lock,val));
REDEF_FUNCTION(int ,futex_wake,(void *lock, int n_to_wake),(lock,n_to_wake));

// #include "sos_intwait.h"

REDEF_FUNCTION(int ,intwait,(unsigned long long int mask),(mask));

// #include "sos_mutex.h"

typedef blast_mutex_t sos_mutex_t;

REDEF_FUNCTION(void ,mutex_lock,(blast_mutex_t *lock),(lock));		/* blocking */
REDEF_FUNCTION(void ,mutex_unlock,(blast_mutex_t *lock),(lock));	/* unlock */
REDEF_FUNCTION(int ,mutex_trylock,(blast_mutex_t *lock),(lock));	/* just try... 1 if successful, 0 if not */
REDEF_FUNCTION(void ,mutex_init,(blast_mutex_t *lock),(lock)) 	/* initialize it... */

// #include "sos_pipe.h"

typedef blast_pipe_data_t sos_pipe_data_t;

typedef blast_pipe_t sos_pipe_t;

REDEF_FUNCTION(blast_pipe_t *,pipe_alloc,(unsigned int size_in_bytes),(size_in_bytes));
REDEF_FUNCTION(blast_pipe_t *,pipe_create,(blast_pipe_t *pipe, blast_pipe_data_t *data, int data_elements),(pipe,data,data_elements));
REDEF_FUNCTION(void ,pipe_free,(blast_pipe_t *pipe),(pipe));

REDEF_FUNCTION(void ,pipe_send,(blast_pipe_t *pipe, blast_pipe_data_t data),(pipe,data));
REDEF_FUNCTION(blast_pipe_data_t, pipe_recv,(blast_pipe_t *pipe),(pipe));

REDEF_FUNCTION(int ,pipe_trysend,(blast_pipe_t *pipe, blast_pipe_data_t data),(pipe,data));
REDEF_FUNCTION(blast_pipe_data_t, pipe_tryrecv,(blast_pipe_t *pipe, int *success),(pipe,success));

// #include "sos_printf.h"

// REDEF_FUNCTION(int ,printf,(const char *fmt, ...));
#define sos_printf(...) blast_printf(__VA_ARGS__)

// #include "sos_thread.h"

REDEF_FUNCTION(int ,thread_create,(void *pc, void *stack, void *arg, unsigned int prio, unsigned int asid),(pc,stack,arg,prio,asid));
REDEF_FUNCTION(void ,thread_stop,(),());
REDEF_FUNCTION(int ,thread_myid,(),());

// #include "sos_trace.h"

#ifndef SOS_DEBUG
#define SOS_TRACE(str, ...) __VA_ARGS__
#else
#define SOS_TRACE(str, ...) \
	do { \
		blast_printf("%s:%d: %s: >>> calling %s\n",__FILE__,__LINE__,str,#__VA_ARGS__); \
		__VA_ARGS__; \
		blast_printf("%s:%d: %s: <<< %s returned\n",__FILE__,__LINE__,str,#__VA_ARGS__); \
	} while (0);
#endif

// #include "sos_cycles.h"
REDEF_FUNCTION(unsigned long long int ,get_pcycles,(),());

REDEF_FUNCTION(unsigned long long int ,get_tcycles,(),());

// #include "sos_sem.h"
typedef blast_sem_t sos_sem_t;

REDEF_FUNCTION(int ,sem_add,(blast_sem_t *sem, unsigned int amt),(sem,amt));
REDEF_FUNCTION(int ,sem_up,(blast_sem_t *sem),(sem));
REDEF_FUNCTION(int ,sem_down,(blast_sem_t *sem),(sem));
REDEF_FUNCTION(int ,sem_trydown,(blast_sem_t *sem),(sem));
REDEF_FUNCTION(void ,sem_init,(blast_sem_t *sem),(sem));

// #include "sos_barrier.h"

typedef blast_barrier_t sos_barrier_t;

#define BLAST_BARRIER_SERIAL_THREAD 1
#define BLAST_BARRIER_OTHER 0

REDEF_FUNCTION(int ,barrier_init,(blast_barrier_t *barrier, unsigned int threads_total),(barrier,threads_total));
REDEF_FUNCTION(int ,barrier_wait,(blast_barrier_t *barrier),(barrier));

// #include "sos_libkernel.h"
REDEF_FUNCTION(void ,init,(unsigned long long int *memmap),(memmap));
// #include "sos_fastint.h"
REDEF_FUNCTION(void ,register_fastint,(int intno, void (*fn)(int)),(intno,fn));

// #include "sos_allsignal.h"

typedef blast_allsignal_t sos_allsignal_t;
REDEF_FUNCTION(void ,allsignal_init,(blast_allsignal_t *signal),(signal));
REDEF_FUNCTION(void ,allsignal_wait,(blast_allsignal_t *signal, unsigned int mask),(signal,mask));
REDEF_FUNCTION(void ,allsignal_signal,(blast_allsignal_t *signal, unsigned int mask),(signal,mask));

#endif
