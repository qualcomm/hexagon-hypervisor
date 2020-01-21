/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <pthread.h>
#include <pthread_internal_misc.h>
#include <h2if.h>
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
	/* FIXME: four semaphores? Come on. You can do better. 
	 * Maybe use the same barrier twice?
	 */
	pthread_sem_t joined;
	pthread_sem_t waiters;
	pthread_sem_t exiting;
	pthread_sem_t ack;
	pthread_attr_t attrs;
};

extern char TLS_START __attribute__((weak));
extern char TLS_END __attribute__((weak));

static char *elftls_start;
static unsigned int elftls_size;

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

static void *old_freeptr = NULL;
static void *old_stack_freeptr = NULL;
static pthread_plainmutex_t pthread_exit_lock = PTHREAD_PLAINMUTEX_INITIALIZER_NP;

void __attribute__((noreturn)) pthread_exit(void *retval)
{
	char *freeptr;
	struct pthread_tcb *self = pthread_self_ptr();
	/* EJP: FIXME? Maybe? If we wake up someone joining us, does that mean that our stack should be available? */
	self->retval = retval;
	pthread_sem_post_np(&self->waiters);
	if (!self->attrs.detached) {
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
	if (!self->attrs.detached) {
		pthread_sem_post_np(&self->exiting);
		pthread_sem_wait_np(&self->ack);
	}
	pthread_safe_death(&pthread_exit_lock,pthread_self());
}

static inline void pthread_special_setup(struct pthread_tcb *self)
{
	/* SET UGP */
	asm volatile (" ugp = %0 " : : "r"(self) : "memory");
	/* FIXME: set up FRAMELIMIT */
}

static void pthread_trampoline(void *arg)
{
	struct pthread_tcb *self = arg;
	void *ret;
	/* before self->id is set up, we have to be extra careful! */
	pthread_special_setup(self); /* Set up UGP = self, init thread-local h2_tid, ... */
	pthread_sem_wait_np(&self->ack);
	/* FIXME: set up FRAMEKEY */
	if (self->attrs.extra_ctor) self->attrs.extra_ctor(self->attrs.extra);
	ret = self->start_routine(self->start_arg);
	pthread_exit(ret);
}

int pthread_detach(pthread_t thread)
{
	struct pthread_tcb *dest;
	if ((dest = pthread_thread_find_id(thread)) == NULL) return ESRCH;
	if (dest->attrs.detached) return EINVAL;
	dest->attrs.detached = 1;
	pthread_sem_post_np(&dest->joined);
	return 0;
}

int pthread_join(pthread_t thread, void **retval)
{
	struct pthread_tcb *dest;
	if ((dest = pthread_thread_find_id(thread)) == NULL) return ESRCH;
	if (dest->attrs.detached) return EINVAL;
	pthread_sem_wait_np(&dest->waiters);
	if (retval) *retval = dest->retval;
	pthread_sem_post_np(&dest->joined);	/* Let thread know we've read values */
	pthread_sem_wait_np(&dest->exiting);	/* Wait for thread to be ready to die */
	pthread_sem_post_np(&dest->ack);	/* Wait for thread to be ready to die */
	pthread_plainmutex_lock_np(&pthread_exit_lock);	/* acquire and release to ensure serialization */
	pthread_plainmutex_unlock_np(&pthread_exit_lock);
	return 0;
}

static struct pthread_tcb *pthread_create_common(struct pthread_tcb *dst)
{
	char *elftls_area;
	/* init semaphores */
	pthread_sem_init_np(&dst->joined,0,0);
	pthread_sem_init_np(&dst->waiters,0,0);
	pthread_sem_init_np(&dst->exiting,0,0);
	pthread_sem_init_np(&dst->ack,0,0);
	/* Copy ELF TLS area */
	elftls_area = (char *)dst;
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
	/* OK, we have this problem:
	 * If we create the thread, it should be joinable.
	 * We can't let the thread put itself in the list of threads, because we might try to join it before it can run.
	 * We can't put the thread structure in the list of threads before we create it, because we don't know the threadid until we create it.
	 * So we might be able to put the thread in the list of threads ourselves right after creation. 
	 * But wait! If the thread we created preempts us, it can't find itself in the list!
	 * 
	 * I see two options:
	 * A) Before thread creation, acquire a mutex that the created thread will also acquire, so that we can guarantee making it into the list, 
	 * B) After thread creation, after the thread has put itself in the list, the new thread posts a semaphore that we wait on before returning
	 * C) After thread creation, bump a semaphore to the child. More concurrent!
	 * I like option B and C better, but option A probably has more priority inversion avoidance?
	 */
	if ((*thread = h2_thread_create_trap(pthread_trampoline,(stackaddr+stacksize), dest, attr->sched.sched_priority)) == -1) {
		/* If error, Handle thread creation error */
		if (self_stack) free(self_stack);
		free(tmpptr);
		return EAGAIN;
	}
	dest->id = *thread;
	pthread_tcb_add(dest);
	pthread_sem_post_np(&dest->ack);	/* Child OK to start */
	return 0;
}

static struct {
	unsigned long long int tls_storage[MAX_ELFTLS_SIZE/8];
	struct pthread_tcb main_tcb;
} mainthread_static_storage;

void pthread_mainthread_setup()
{
	struct pthread_tcb *self = &mainthread_static_storage.main_tcb;
	self->id = h2_thread_myid();
	self->attrs.detached = 1;
	pthread_create_common(self);
	pthread_special_setup(self);
	pthread_tcb_add(self);
}

static void do_pthread_init()
{
	char *elftls_end = &TLS_END;
	elftls_start = &TLS_START;
	elftls_size = elftls_end - elftls_start;
	if (elftls_size > MAX_ELFTLS_SIZE) {
		/* FIXME: FATAL ERROR @ BOOT */;
		elftls_size = 0;
	}
	pthread_mainthread_setup();
}

__attribute__((constructor(0)))
void pthread_init()
{
	static pthread_once_t init_once = PTHREAD_ONCE_INIT;
	pthread_once(&init_once,do_pthread_init);
}

