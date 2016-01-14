/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _H2_PTHREAD_H
#define _H2_PTHREAD_H 1

#ifdef __cplusplus
extern "C" {
#endif

/* EJP: should these point to qurt or h2? 
 * I want to ensure H2 semantics, but pointing to qurt will make this portable there too?
 */

/* #include <h2.h> */
#include <qurt.h>
#include <errno.h>

#define PTHREAD_DEFAULT_STACKSIZE 1024
#define PTHREAD_PROCESS_SHARED 0
#define PTHREAD_PRIO_NONE 0

static inline int pthread_h2_unsup() { return ENOTSUP; }
static inline int pthread_h2_inval() { return EINVAL; }

struct sched_param { 
	int sched_priority;
};

/* PTHREAD */
typedef struct {
	size_t stacksize;
	void *stackaddr;
	struct sched_param sched;
} pthread_attr_t;

typedef unsigned int pthread_t;

static inline int pthread_attr_init(pthread_attr_t *attr)
{
	const pthread_attr_t mydefault = { PTHREAD_DEFAULT_STACKSIZE, NULL, { 100 } };
	*attr = mydefault;
	return 0;
}

static inline int pthread_attr_destroy(pthread_attr_t *attr)
{
	return 0;
}

static inline int pthread_attr_getschedpolicy(pthread_attr_t *attr, int *policy) { return pthread_h2_unsup(); }
static inline int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy) { return pthread_h2_unsup(); }
static inline int pthread_attr_getinheritsched(pthread_attr_t *attr, int *is) { return pthread_h2_unsup(); }
static inline int pthread_attr_setinheritsched(pthread_attr_t *attr, int is) { return pthread_h2_unsup(); }
static inline int pthread_attr_getschedparam(pthread_attr_t *attr, struct sched_param *p) { *p = attr->sched; return 0; }
static inline int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *p) { attr->sched = *p; return 0; }
static inline int pthread_attr_getstacksize(pthread_attr_t *attr, size_t *stacksize) { *stacksize = attr->stacksize; return 0; }
static inline int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize) { attr->stacksize = stacksize; return 0; }
static inline int pthread_attr_getstackaddr(pthread_attr_t *attr, void **stackaddr) { *stackaddr = attr->stackaddr; return 0; }
static inline int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr) { attr->stackaddr = stackaddr; return 0; }
/* EJP: probably others, scope? guardsize? detachstate? */

static inline int pthread_attr_getstack(pthread_attr_t *attr, void **stackaddr, size_t *stacksize)
{
	*stackaddr = attr->stackaddr;
	*stacksize = attr->stacksize;
	return 0;
}

static inline int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize)
{
	attr->stackaddr = stackaddr;
	attr->stacksize = stacksize;
	return 0;
}

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
int pthread_detach(pthread_t thread);
void pthread_exit(void *retval);

static inline int pthread_equal(pthread_t t1, pthread_t t2) { return t1 == t2; }
static inline int pthread_cancel(pthread_t thread) { return pthread_h2_unsup(); }
static inline int pthread_yield(void) { h2_yield(); return 0; }
static inline pthread_t pthread_self(void) { return qurt_thread_get_id(); }
int pthread_join(pthread_t thread, void **retval);

/* MUTEX */

/* EJP: 
 * 
 * Because the QuRT we borrowed from has separated plain and recursive mutexes, we 
 * will use the H2 interface here, which matches POSIX more closely.
 */

typedef struct {
	unsigned int type;
	/* Don't need PSHARED ? */
} pthread_mutexattr_t;

typedef h2_mutex_t pthread_mutex_t;

#define PTHREAD_MUTEX_INITIALIZER QURT_MUTEX_INIT
#define PTHREAD_MUTEX_NORMAL H2_MUTEX_PLAIN
#define PTHREAD_MUTEX_ERRORCHECK PTHREAD_MUTEX_NORMAL /* NOT SUPPORTED */
#define PTHREAD_MUTEX_DEFAULT PTHREAD_MUTEX_NORMAL
#define PTHREAD_MUTEX_RECURSIVE H2_MUTEX_RECURSIVE

static inline int pthread_mutexattr_init(pthread_mutexattr_t *attr) { attr->type = PTHREAD_MUTEX_NORMAL; return 0; }
static inline int pthread_mutexattr_destroy(pthread_mutexattr_t *attr) { return 0; }
static inline int pthread_mutexattr_gettype(pthread_mutexattr_t *attr, int *type) { *type = attr->type; return 0; }
static inline int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type) { attr->type = type; return 0; }
static inline int pthread_mutexattr_getpshared(pthread_mutexattr_t *attr, int *pshared) { *pshared = PTHREAD_PROCESS_SHARED; return 0; }
static inline int pthread_mutexattr_setpshared(pthread_mutexattr_t *attr, int pshared) { return 0; }
static inline int pthread_mutexattr_getprotocol(pthread_mutexattr_t *attr, int *protocol) { *protocol = PTHREAD_PRIO_NONE; return 0; }
static inline int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol) { return 0; }
static inline int pthread_mutexattr_getprioceiling(pthread_mutexattr_t *attr, int *ceil) { *ceil = 0; return 0; }
static inline int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int ceil) { return 0; }

static inline int pthread_mutex_destroy(pthread_mutex_t *mutex) { return 0; }
static inline int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr) { h2_mutex_init_type(mutex,attr->type); return 0; }

static inline int pthread_mutex_lock(pthread_mutex_t *mutex) { h2_mutex_lock(mutex); return 0; }
static inline int pthread_mutex_trylock(pthread_mutex_t *mutex) { return h2_mutex_trylock(mutex); }
static inline int pthread_mutex_unlock(pthread_mutex_t *mutex) { h2_mutex_unlock(mutex); return 0; }
static inline int pthread_mutex_setprioceiling(pthread_mutex_t *mutex, int ceil, int *oldceil) { *oldceil = 0; return 0; }
static inline int pthread_mutex_getprioceiling(pthread_mutex_t *mutex, int *ceil)  { *ceil = 0; return 0; }

/* COND */

typedef struct {
	int unused;
} pthread_condattr_t;

static inline int pthread_condattr_init(pthread_condattr_t *attr) { attr->unused = 0; return 0; }
static inline int pthread_condattr_destroy(pthread_condattr_t *attr) { return 0; }
static inline int pthread_condattr_getpshared(pthread_condattr_t *attr, int *pshared) { *pshared = PTHREAD_PROCESS_SHARED; return 0; }
static inline int pthread_condattr_setpshared(pthread_condattr_t *attr, int pshared) { return 0; }

typedef qurt_cond_t pthread_cond_t;
#define PTHREAD_COND_INITIALIZER { .raw = 0 }

static inline int pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attr) { qurt_cond_init(cond); return 0; }
static inline int pthread_cond_destroy(pthread_cond_t *cond) { return 0; }
static inline int pthread_cond_broadcast(pthread_cond_t *cond) { qurt_cond_broadcast(cond); return 0; }
static inline int pthread_cond_signal(pthread_cond_t *cond) { qurt_cond_signal(cond); return 0; }
static inline int pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex) { qurt_cond_wait(cond,mutex); return 0; }

/* BARRIER */

#define PTHREAD_BARRIER_SERIAL_THREAD QURT_BARRIER_SERIAL_THREAD

typedef struct { 
	int unused;
} pthread_barrierattr_t;

typedef qurt_barrier_t pthread_barrier_t;

static inline int pthread_barrierattr_init(pthread_barrierattr_t *attr) { attr->unused = 0; return 0; }
static inline int pthread_barrierattr_destroy(pthread_barrierattr_t *attr) { return 0; }
static inline int pthread_barrierattr_setpshared(pthread_barrierattr_t *attr, int pshared) { return 0; }
static inline int pthread_barrierattr_getpshared(pthread_barrierattr_t *attr, int *pshared) { *pshared = PTHREAD_PROCESS_SHARED; return 0; }

static inline int pthread_barrier_init(pthread_barrier_t *barrier, const pthread_barrierattr_t *attr, unsigned count) { return qurt_barrier_init(barrier,count); }
static inline int pthread_barrier_destroy(pthread_barrier_t *barrier) { return 0; }
static inline int pthread_barrier_wait(pthread_barrier_t *barrier) { return qurt_barrier_wait(barrier); }

/* MISC */

typedef unsigned int pthread_once_t;
#define PTHREAD_ONCE_INIT 0

typedef unsigned int sigset_t;

int pthread_once(pthread_once_t *once_control, void (*init_routine)(void));
static inline int pthread_getconcurrency(int new_level) { return 0; }
static inline int pthread_setconcurrency(void) { return 0; }
int pthread_kill(pthread_t threawd, int sig);
int pthread_sigmask(int how, const sigset_t *set, sigset_t *oldset);

/* TLS */

typedef int pthread_key_t;

static inline void *pthread_getspecific(pthread_key_t key) { return qurt_tls_get_specific(key); }
static inline int pthread_setspecific(pthread_key_t key, const void *value) { return qurt_tls_set_specific(key,value); }
static inline int pthread_key_create(pthread_key_t *key, void (*destructor)(void *)) { return qurt_tls_create_key(key,destructor); }
static inline int pthread_key_delete(pthread_key_t key) { return qurt_tls_delete_key(key); }

#ifdef __cplusplus
}; /* extern "C" */
#endif

#endif

