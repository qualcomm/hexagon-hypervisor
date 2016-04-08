/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _SYS_TYPES_H_
#define _SYS_TYPES_H_

#include <stddef.h>
#include <qurt.h>

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

/* mutex and cond_var shared */
#define PTHREAD_PROCESS_PRIVATE      0
#define PTHREAD_PROCESS_SHARED       1

/* mutex type */
#define PTHREAD_MUTEX_ERRORCHECK     0
#define PTHREAD_MUTEX_NORMAL         1
#define PTHREAD_MUTEX_RECURSIVE      2

/* mutex protocol */
#define PTHREAD_PRIO_NONE            0
#define PTHREAD_PRIO_INHERIT         1
#define PTHREAD_PRIO_PROTECT         2

/*  
 *  The POSIX specification ( http://www.opengroup.org/onlinepubs/009695399/basedefs/pthread.h.html )
 *  states that static initialization with PTHREAD_?_INITIALIZER and 
 *  calling pthread_?_init() should result in the same thing.
 *
 *  If the OS needs additional notification/initialization, that should probably
 *  be handled elsewhere, preferably in a manner that doesn't screw up the basic 
 *  implementation.  H2 doesn't currently need this anyways.
 *
 *  (Some implementations will detect if the ? has been "truly" initialized 
 *  in pthread_?_(lock/wait) and take a "slow path" if not.) 
 * 
 *  (You'd think you would do it just by setting is_initialized to false in both
 *  PTHREAD_?_INITIALIZER and the pthread_?_init() function, and force
 *  pthread_?_(lock/wait) to ALWAYS finalize the initialization at first call.  
 *  I mean, why would you have an is_initialized in the struct and not actually 
 *  use it for that purpose?)
 */

typedef struct 
{
//    int is_initialized;  Not needed for H2.
    int type;
    int pshared;
    int protocol;
} pthread_mutexattr_t;

/*  The default should be:  recursive, private, inherit  */
#define PTHREAD_MUTEXATTR_T_INIT { PTHREAD_MUTEX_RECURSIVE, \
	PTHREAD_PROCESS_PRIVATE, PTHREAD_PRIO_INHERIT }

/*  
 *  In QURT the two data types were the same, but in H2 they're different.
 *  This is a little clunky, but at least it should work and be readable.
 *  Not using a union for now so I can totally statically init everything.
 *  Might be excessive.  Like my comments.
 */

typedef struct {
	pthread_mutexattr_t attributes;
	h2_mutex_t  mutex;
	h2_rmutex_t rmutex;
} pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER { PTHREAD_MUTEXATTR_T_INIT, \
	H2_MUTEX_T_INIT, H2_RMUTEX_T_INIT }

#define PTHREAD_SPINLOCK_UNLOCKED    0
#define PTHREAD_SPINLOCK_LOCKED      1

/*
 *  What?  Are you sure you don't want to make this datatype an integer 
 *  which is used as a pointer to a struct that contains an integer?  
 */

typedef unsigned int              pthread_spinlock_t;

#define PTHREAD_SPINLOCK_T_INIT PTHREAD_SPINLOCK_UNLOCKED

typedef struct pthread_condattr_t
{
    int is_initialized;
    int pshared;
} pthread_condattr_t;

typedef unsigned int             pthread_cond_t;

#define PTHREAD_COND_INITIALIZER     ((pthread_cond_t) 0xFFFFFFFF)

typedef struct _pthread_cond_t   _pthread_cond_t;
struct _pthread_cond_t
{
    pthread_condattr_t attr;
    qurt_cond_t       *qurt_cond;
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
    qurt_barrier_t       *qurt_barrier;
};

typedef long       off_t;

typedef long int   time_t;
#define _TIME_T

#endif /* _POSIX_SCHED_H_ */
