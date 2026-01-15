/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef PTHREAD_THREAD_H
#define PTHREAD_THREAD_H 1

#define PTHREAD_DEFAULT_STACKSIZE 8192
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

int pthread_attr_init(pthread_attr_t *attr);
int pthread_attr_destroy(pthread_attr_t *attr);
int pthread_attr_getschedpolicy(pthread_attr_t *attr, int *policy);
int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy);
int pthread_attr_getinheritsched(pthread_attr_t *attr, int *is);
int pthread_attr_setinheritsched(pthread_attr_t *attr, int is);
int pthread_attr_getextra_np(pthread_attr_t *attr, void **extra, void (**constructor)(void *), void (**destructor)(void *));
int pthread_attr_setextra_np(pthread_attr_t *attr, void *extra, void (*constructor)(void *), void (*destructor)(void *));

enum {
	PTHREAD_CREATE_JOINABLE = 0,
	PTHREAD_CREATE_DETACHED = 1,
};

int pthread_attr_getdetachstate(pthread_attr_t *attr, int *ds);
int pthread_attr_setdetachstate(pthread_attr_t *attr, int ds);
int pthread_attr_getschedparam(pthread_attr_t *attr, struct sched_param *p);
int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *p);
int pthread_attr_getstacksize(pthread_attr_t *attr, size_t *stacksize);
int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize);
int pthread_attr_getstackaddr(pthread_attr_t *attr, void **stackaddr);
int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr);
/* EJP: probably others, scope? guardsize? */

int pthread_attr_getstack(pthread_attr_t *attr, void **stackaddr, size_t *stacksize);
int pthread_attr_setstack(pthread_attr_t *attr, void *stackaddr, size_t stacksize);
pthread_t pthread_self(void);

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);
int pthread_detach(pthread_t thread);
void pthread_exit(void *retval) __attribute__((noreturn));

int pthread_equal(pthread_t t1, pthread_t t2);
int pthread_cancel(pthread_t thread);
int pthread_yield(void);
int pthread_join(pthread_t thread, void **retval);

void pthread_init();

#endif
