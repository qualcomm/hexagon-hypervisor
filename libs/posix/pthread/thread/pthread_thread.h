/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef PTHREAD_THREAD_H
#define PTHREAD_THREAD_H 1

#define PTHREAD_DEFAULT_STACKSIZE 1024
#define PTHREAD_PROCESS_SHARED 0
#define PTHREAD_PRIO_NONE 0

/* PTHREAD */
typedef struct {
	size_t stacksize;
	void *stackaddr;
	struct sched_param sched;
	int detached;
	void *extra;
	void (*extra_ctor)(void *);
	void (*extra_dtor)(void *);
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

static inline int pthread_attr_getschedpolicy(pthread_attr_t *attr, int *policy) { return ENOTSUP; }
static inline int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy) { return ENOTSUP; }
static inline int pthread_attr_getinheritsched(pthread_attr_t *attr, int *is) { return ENOTSUP; }
static inline int pthread_attr_setinheritsched(pthread_attr_t *attr, int is) { return ENOTSUP; }

static inline int pthread_attr_getextra_np(pthread_attr_t *attr, 
	void **extra, 
	void (**constructor)(void *), 
	void (**destructor)(void *))
{
	*extra = attr->extra;
	*constructor = attr->extra_ctor;
	*destructor = attr->extra_dtor;
	return 0;
}

static inline int pthread_attr_setextra_np(pthread_attr_t *attr, 
	void *extra, 
	void (*constructor)(void *), 
	void (*destructor)(void *))
{
	attr->extra = extra;
	attr->extra_ctor = constructor;
	attr->extra_dtor = destructor;
	return 0;
}

static inline int pthread_attr_getdetachstate(pthread_attr_t *attr, int *ds) { *ds = attr->detached; return 0; }
static inline int pthread_attr_setdetachstate(pthread_attr_t *attr, int ds) { attr->detached = ds; return 0; }
static inline int pthread_attr_getschedparam(pthread_attr_t *attr, struct sched_param *p) { *p = attr->sched; return 0; }
static inline int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *p) { attr->sched = *p; return 0; }
static inline int pthread_attr_getstacksize(pthread_attr_t *attr, size_t *stacksize) { *stacksize = attr->stacksize; return 0; }
static inline int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize) { attr->stacksize = stacksize; return 0; }
static inline int pthread_attr_getstackaddr(pthread_attr_t *attr, void **stackaddr) { *stackaddr = attr->stackaddr; return 0; }
static inline int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr) { attr->stackaddr = stackaddr; return 0; }
/* EJP: probably others, scope? guardsize? */

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

static inline pthread_t pthread_self(void) { pthread_t *idp; asm (" %0 = ugp " : "=r"(idp)); return *idp; }

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
int pthread_detach(pthread_t thread);
void pthread_exit(void *retval) __attribute__((noreturn));

static inline int pthread_equal(pthread_t t1, pthread_t t2) { return t1 == t2; }
static inline int pthread_cancel(pthread_t thread) { return ENOTSUP; }
static inline int pthread_yield(void) { h2_yield(); return 0; }
int pthread_join(pthread_t thread, void **retval);

#endif
