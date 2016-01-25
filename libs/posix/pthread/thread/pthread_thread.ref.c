/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>
#include <pthread_internal_misc.h>
#include <h2.h>
#include <string.h>
#include <stdio.h>

/* EJP: should pthread_self return H2 TID or pthread TCB? */
/* pthread tcb is in UGP, we can make h2 tid a ELFTLS variable */

#define MAX_ELFTLS_SIZE 256

struct pthread_tcb {
	pthread_t id;				/* MUST BE AT ZERO */
	void **pthread_tls_ptr;			/* MUST BE AT FOUR */
	struct pthread_tcb *next;
	void *(*start_routine)(void *);
	void *start_arg;
	void *stack_free;
	void *retval;
	pthread_sem_t joined;
	pthread_sem_t waiters;
	pthread_attr_t attrs;
};

extern char tdata_start;
extern char tdata_end;
extern char tbss_start;
extern char tbss_end;

static char *elftls_start;
static char *elftls_end;
static unsigned int elftls_size;

__thread unsigned int __pthread_thread_dummy = 1;

static struct pthread_tcb *pthread_root = NULL;
static pthread_plainmutex_t pthread_tcb_mutex = PTHREAD_PLAINMUTEX_INITIALIZER_NP;

static inline struct pthread_tcb *pthread_self_ptr()
{
	struct pthread_tcb *ret;
	asm volatile (" %0 = ugp " : "=r"(ret) );
	return ret;
}

static inline void pthread_tcb_add(struct pthread_tcb *dst)
{
	pthread_plainmutex_lock_np(&pthread_tcb_mutex);
	dst->next = pthread_root;
	pthread_root = dst;
	pthread_plainmutex_unlock_np(&pthread_tcb_mutex);
}

static inline void pthread_tcb_do_remove(struct pthread_tcb **root, struct pthread_tcb *dst)
{
	if (*root == NULL) return;	/* NOT FOUND */
	if (*root == dst) {
		*root = dst->next;
	} else {
		pthread_tcb_do_remove(&((*root)->next),dst);
	}
}

static inline void pthread_tcb_remove(struct pthread_tcb *dst)
{
	pthread_plainmutex_lock_np(&pthread_tcb_mutex);
	pthread_tcb_do_remove(&pthread_root,dst);
	pthread_plainmutex_unlock_np(&pthread_tcb_mutex);
}

static struct pthread_tcb *pthread_thread_find_id(pthread_t id)
{
	struct pthread_tcb *ptr;
	pthread_plainmutex_lock_np(&pthread_tcb_mutex);
	for (ptr = pthread_root; (ptr != NULL) && (ptr->id != id); ptr = ptr->next) /* find */;
	pthread_plainmutex_unlock_np(&pthread_tcb_mutex);
	return ptr;
}

/* 
 * EJP: FIXME optimization: Instead of allocating twice, one for tcb+tls and one for stack, why not one allocation with both?  
 * Then we only need to free one thing.
 */

void __attribute__((noreturn)) pthread_exit(void *retval)
{
	char *freeptr;
	struct pthread_tcb *self = pthread_self_ptr();
	static void *old_freeptr = NULL;
	static void *old_stack_freeptr = NULL;
	static pthread_plainmutex_t pthread_exit_lock = PTHREAD_PLAINMUTEX_INITIALIZER_NP;
	if (!self->attrs.detached) {
		self->retval = retval;
		pthread_sem_post_np(&self->waiters);
		pthread_sem_wait_np(&self->joined);
	}
	if (self->attrs.extra_dtor) self->attrs.extra_dtor(self->attrs.extra);
	pthread_tcb_remove(self);
	pthread_tls_teardown();
	freeptr = (char *)self;
	freeptr -= elftls_size;
	pthread_plainmutex_lock_np(&pthread_exit_lock);
	if (old_freeptr) free(old_freeptr);
	if (old_stack_freeptr) free(old_stack_freeptr);
	old_freeptr = freeptr;
	old_stack_freeptr = self->stack_free;
	pthread_safe_death(&pthread_exit_lock,pthread_self());
}

static inline void pthread_special_setup(struct pthread_tcb *self)
{
	/* SET UGP */
	self->id = h2_thread_myid();
	asm volatile (" ugp = %0 " : : "r"(self) : "memory");
	/* FIXME: set up FRAMELIMIT */
}

static void pthread_trampoline(void *arg)
{
	struct pthread_tcb *self = arg;
	void *ret;
	pthread_special_setup(self); /* Set up UGP = self, init thread-local h2_tid, ... */
	pthread_tcb_add(self);
	/* FIXME: set up FRAMEKEY */
	if (self->attrs.extra_ctor) self->attrs.extra_ctor(self->attrs.extra);
	ret = self->start_routine(self->start_arg);
	pthread_exit(ret);
}

int pthread_join(pthread_t thread, void **retval)
{
	struct pthread_tcb *dest;
	if ((dest = pthread_thread_find_id(thread)) == NULL) return ESRCH;
	if (dest->attrs.detached) return EINVAL;
	pthread_sem_wait_np(&dest->waiters);
	*retval = dest->retval;
	pthread_sem_post_np(&dest->joined);
	return 0;
}

static struct pthread_tcb *pthread_create_common(struct pthread_tcb *dst)
{
	unsigned char *elftls_area;
	/* init semaphores */
	pthread_sem_init_np(&dst->joined,0,0);
	pthread_sem_init_np(&dst->waiters,0,0);
	/* Copy ELF TLS area */
	elftls_area = (unsigned char *)dst;
	elftls_area -= elftls_size;
	memcpy(elftls_area,elftls_start,elftls_size);
	return dst;
}

static const pthread_attr_t pthread_create_default_attr = { PTHREAD_DEFAULT_STACKSIZE, NULL, { 100 } };

int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg)
{
	struct pthread_tcb *dest;
	unsigned char *stackaddr;
	unsigned char *self_stack = NULL;
	unsigned char *tmpptr;
	size_t stacksize;
	if (attr == NULL) {
		attr = &pthread_create_default_attr;
	}
	/* If necessary, allocate stack and set dest->stack_free */
	stackaddr = (unsigned char *)((unsigned long)attr->stackaddr & -8);
	stacksize = attr->stacksize;
	if (stackaddr == NULL) {
		if ((stackaddr = malloc(stacksize)) == NULL) {
			return EAGAIN;
		}
		self_stack = stackaddr;
	}
	if ((tmpptr = calloc(1,elftls_size + sizeof(struct pthread_tcb))) == NULL) {
		if (self_stack) free(self_stack);
		return EAGAIN;
	}
	dest = (struct pthread_tcb *)(tmpptr + elftls_size);
	pthread_create_common(dest);
	/* Create H2 thread */
	dest->attrs = *attr;
	dest->stack_free = self_stack;
	if (self_stack) dest->attrs.stackaddr = self_stack;
	dest->start_arg = arg;
	dest->start_routine = start_routine;
	if ((*thread = h2_thread_create(pthread_trampoline,(stackaddr+stacksize), dest, attr->sched.sched_priority)) == -1) {
		/* If error, Handle thread creation error */
		if (self_stack) free(self_stack);
		free(tmpptr);
		return EAGAIN;
	}
	return 0;
}

static struct {
	unsigned long long int tls_storage[MAX_ELFTLS_SIZE/8];
	struct pthread_tcb main_tcb;
} mainthread_static_storage;

void pthread_mainthread_setup()
{
	struct pthread_tcb *self = &mainthread_static_storage.main_tcb;
	pthread_create_common(self);
	pthread_special_setup(self);
	pthread_tcb_add(self);
}

static void do_pthread_init()
{
	elftls_size = ((((&tbss_end-&tbss_start) + (&tdata_end - &tdata_start))+7) & -8);
	elftls_start = &tdata_start;
	elftls_end = elftls_start + elftls_size;
	if (elftls_size > MAX_ELFTLS_SIZE) /* FIXME: FATAL ERROR @ BOOT */;
	pthread_mainthread_setup();
}

void pthread_init()
{
	static pthread_once_t init_once = PTHREAD_ONCE_INIT;
	pthread_once(&init_once,do_pthread_init);
}

