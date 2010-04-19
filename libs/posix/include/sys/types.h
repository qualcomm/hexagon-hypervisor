/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

#include <stddef.h>
#include <blast.h>

#define PTHREAD_MUTEX_OPAQUE

#ifdef __GNUC__
#define restrict __restrict__
#else
#define restrict
#endif

#define PTHREAD_MAX_THREADS          512

#define PTHREAD_NAME_LEN             16
#define PTHREAD_MIN_STACKSIZE        512 //4096
#define PTHREAD_MAX_STACKSIZE        1048576
#define PTHREAD_DEFAULT_STACKSIZE    1024

#define PTHREAD_MIN_PRIORITY         0
#define PTHREAD_MAX_PRIORITY         255
#define PTHREAD_DEFAULT_PRIORITY     20

typedef signed int   ssize_t;
#define _SSIZE_T

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE    0
#endif

typedef unsigned int cpu_set_t;

typedef unsigned int pthread_t;
typedef struct pthread_attr_t
{
    void         *stackaddr;
    int          internal_stack; /* this flag==1 means the stack needs to be freed by posix */
    size_t       stacksize;
    int          priority;
    unsigned int timetest_id;    
    cpu_set_t    cpumask;
    char         name[PTHREAD_NAME_LEN];
} pthread_attr_t;

#define PTHREAD_MUTEX_ATTR_UNINITIALIZED    0
#define PTHREAD_MUTEX_ATTR_INITIALIZED      1

#define PTHREAD_COND_ATTR_UNINITIALIZED     0
#define PTHREAD_COND_ATTR_INITIALIZED       1

#define PTHREAD_DEFAULT_NAME                "Anonymous"

#ifdef PTHREAD_MUTEX_OPAQUE
#define PTHREAD_MUTEX_INITIALIZER    ((pthread_mutex_t) 0xFFFFFFFF)

#else
#define PTHREAD_MUTEX_INITIALIZER    {(blast_mutex_t*)0, \
                                      blast_rmutex_lock, \
                                      blast_rmutex_unlock, \
                                      blast_rmutex_trylock, \
                                      {PTHREAD_MUTEX_ATTR_INITIALIZED, \
                                       PTHREAD_MUTEX_RECURSIVE, \
                                       PTHREAD_PROCESS_PRIVATE, \
                                       PTHREAD_PRIO_INHERIT} \
                                      }
#endif
                                      
#define PTHREAD_COND_INITIALIZER     ((pthread_cond_t) 0xFFFFFFFF)

/* mutex and cond_var shared */
#define PTHREAD_PROCESS_PRIVATE      0
#define PTHREAD_PROCESS_SHARED       1

/* mutex type */
#define PTHREAD_MUTEX_ERRORCHECK     0
#define PTHREAD_MUTEX_NORMAL         1
#define PTHREAD_MUTEX_RECURSIVE      2

#define PTHREAD_MUTEX_DEFAULT        PTHREAD_MUTEX_RECURSIVE

/* mutex protocol */
#define PTHREAD_PRIO_NONE            0
#define PTHREAD_PRIO_INHERIT         1
#define PTHREAD_PRIO_PROTECT         2

//mutex attr
typedef struct pthread_mutexattr_t   pthread_mutexattr_t;
struct pthread_mutexattr_t
{
    int is_initialized;
    int type;
    int pshared;
    int protocol;
};

#ifdef PTHREAD_MUTEX_OPAQUE

//  In BLAST the two data types were the same, but in H2 they're different.

typedef struct {
	union {
		h2_rmutex_t rmutex;
		h2_mutex_t  mutex;
	};
} h2_pthread_mutex_t;

typedef unsigned int              pthread_mutex_t;  //  this is used as a pointer.
typedef struct _pthread_mutex_t   _pthread_mutex_t;

struct _pthread_mutex_t
{
    pthread_mutexattr_t attr;
    h2_pthread_mutex_t	*mutex;		/* holding blast mutex or rmutex pointer */
    void                (*lock)(void *);   /* the function pointer for lock */
    void                (*unlock)(void *); /* the function pointer for unlock */
    int                 (*trylock)(void *);/* the function pointer for trylock */
};

#else
typedef struct pthread_mutex_t
{
    blast_mutex_t       *kmutex;                    /* holding kernel mutex (blast mutex or rmutex) pointer */
    void                (*lock)(blast_mutex_t *);   /* the function pointer for lock */
    void                (*unlock)(blast_mutex_t *); /* the function pointer for unlock */
    int                 (*trylock)(blast_mutex_t *);/* the function pointer for trylock */
    pthread_mutexattr_t attr;
}pthread_mutex_t;
#endif

#define PTHREAD_SPINLOCK_UNLOCKED    0
#define PTHREAD_SPINLOCK_LOCKED      1

typedef unsigned int              pthread_spinlock_t;

typedef struct pthread_condattr_t
{
    int is_initialized;
    int pshared;
} pthread_condattr_t;

typedef unsigned int             pthread_cond_t;

typedef struct _pthread_cond_t   _pthread_cond_t;
struct _pthread_cond_t
{
    pthread_condattr_t attr;
    blast_cond_t       *blast_cond;
};

typedef struct pthread_barrierattr_t
{
    int is_initialized;
    int pshared;
} pthread_barrierattr_t;

typedef unsigned int                pthread_barrier_t;

typedef struct _pthread_barrier_t   _pthread_barrier_t;
struct _pthread_barrier_t
{
    pthread_barrierattr_t attr;
    blast_barrier_t       *blast_barrier;
};

typedef long       off_t;

typedef long int   time_t;
#define _TIME_T

#endif /* _POSIX_SCHED_H_ */
