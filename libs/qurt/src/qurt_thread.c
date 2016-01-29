/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt.h>
#include <pthread.h>
#include <qurt_tls.h>
#include <qurt_thread.h>

#include <string.h>
#include <stdio.h>

static struct QURT_ugp_ptr *qurt_root = NULL;
static h2_mutex_t qurt_root_mutex = H2_MUTEX_T_INIT;

unsigned int curr_futex_queue_used = 0; /* EJP: probably vestigal, but referenced by modem fw */

#if 0
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF(...) /* nothing */
#endif

static struct QURT_ugp_ptr *find_thread(int threadid)
{
	struct QURT_ugp_ptr *tmp;
	h2_mutex_lock(&qurt_root_mutex);
	tmp = qurt_root;
	while ((tmp != NULL) && (tmp->utcb.thread_id != threadid)) tmp = tmp->next;
	h2_mutex_unlock(&qurt_root_mutex);
	return tmp;
}

static void remove_thread(struct QURT_ugp_ptr *ptr)
{
	struct QURT_ugp_ptr **tmp;
	h2_mutex_lock(&qurt_root_mutex);
	for (tmp = &qurt_root; (*tmp != NULL) && (*tmp != ptr); tmp = &((*tmp)->next)) /* FIND THREAD */;
	if (*tmp != NULL) *tmp = ptr->next;
	h2_mutex_unlock(&qurt_root_mutex);
}

static void add_thread(struct QURT_ugp_ptr *ptr)
{
	h2_mutex_lock(&qurt_root_mutex);
	ptr->next = qurt_root;
	qurt_root = ptr;
	h2_mutex_unlock(&qurt_root_mutex);
	DEBUG_PRINTF("Added thread %x\n",h2_thread_myid());
}

void qurt_thread_attr_init (qurt_thread_attr_t *attr)
{
	attr->name[0] = 0;
	attr->tcb_partition = QURT_THREAD_ATTR_TCB_PARTITION_DEFAULT;
	attr->asid = QURT_THREAD_ATTR_ASID_DEFAULT;
	attr->affinity = QURT_THREAD_ATTR_AFFINITY_DEFAULT;
	attr->bus_priority = QURT_THREAD_ATTR_BUS_PRIO_DEFAULT;
	attr->timetest_id = QURT_THREAD_ATTR_TIMETEST_ID_DEFAULT;
	pthread_attr_init(&attr->pthread_attrs);
	qurt_thread_attr_set_priority(attr,QURT_THREAD_ATTR_PRIORITY_DEFAULT);
}

#if 0
/* from qurt_thread.c */
void qurt_trampoline (void *arg)
{
	struct QURT_ugp_ptr *pUgp;

	pUgp = (struct QURT_ugp_ptr *)arg;

	/* Initialize UGP and UTCB variables */
	qurt_thread_initial_setup (pUgp);

	/* Enter thread entry */
	pUgp->utcb.entrypoint (pUgp->utcb.arg);
	printf("QURT TRAMPOLINE: Death of thread %x\n",h2_thread_myid());
	qurt_thread_exit(0);
}
#endif

static void qurt_thread_constructor(void *extra)
{
	struct QURT_ugp_ptr *ugpptr;
	ugpptr = extra;
	ugpptr->utcb.thread_id = pthread_self();
	add_thread(ugpptr);
}

static void qurt_thread_destructor(void *extra)
{
	remove_thread(extra);
	qurt_free(extra);
}

int qurt_thread_create(qurt_thread_t *thread_id, qurt_thread_attr_t *attr, void (*entrypoint) (void *), void *arg)
{
	/*
	 * EJP: now with pthready goodness.
	 * Allocate extra structure
	 * Set extra in pthread attr (w/ appropriate ctor and dtor)
	 * Set qurt trampoline as entry (or use ctor?)
	 * pthread_create
	 * if pthread_create fail, recover and return failure.
	 */
	int pt_ret;
	pthread_t child;
	struct QURT_ugp_ptr *pUgp;

	/* from qurt_thread.c */
	if ((pUgp = qurt_calloc(1,sizeof(struct QURT_ugp_ptr))) == NULL) {
		return QURT_EFATAL;
	}
	pthread_attr_setextra_np(&attr->pthread_attrs,pUgp,qurt_thread_constructor,qurt_thread_destructor);
	memcpy(&pUgp->utcb.attr,attr,sizeof(*attr));
	if ((pt_ret = pthread_create(&child,&attr->pthread_attrs,(void *)entrypoint,arg)) != 0) {
		free(pUgp);
	} else {
		*thread_id = (qurt_thread_t)child;
	}
	return (pt_ret == 0) ? QURT_EOK : QURT_EFATAL;
}

#define BLAST_STACK_SIZE (1024*1024)
/* EJP: fixme: probably remove */
int blast_thread_create(void *pc, void *stack, void *arg, 
	unsigned int prio, unsigned int asid, unsigned int hw_bitmask) {

	qurt_thread_attr_t attr;
	qurt_thread_t id = 0;

	qurt_thread_attr_init(&attr);
	qurt_thread_attr_set_priority(&attr,prio);
	qurt_thread_attr_set_stack_addr(&attr,(unsigned char *)stack-BLAST_STACK_SIZE);	/* FIXME: is that right? */
	qurt_thread_attr_set_stack_size(&attr,BLAST_STACK_SIZE);	/* FIXME: is that right? */

	qurt_thread_create(&id, &attr, pc, arg);
	return id;
}

/* Have a thread become a qurt thread: put it in all the data structures and allocate ugp stuff */
/*
 * EJP: note that we shouldn't malloc here, because we might call this before malloc is OK.
 * But... somehow we need to allocate storage for TLS and copy over the TLS data.
 */
static struct QURT_ugp_ptr main_qurttcb_storage;
void qurt_thread_mainthread_become()
{
	struct QURT_ugp_ptr *pUgp = &main_qurttcb_storage;
	strlcpy(pUgp->utcb.attr.name,"main",QURT_THREAD_ATTR_NAME_MAXLEN);
	pUgp->utcb.thread_id = h2_thread_myid();
	/* FIXME: fill in name and all that stuff? */
	add_thread(pUgp);
}

int qurt_thread_join(unsigned int threadid, int *status)
{
	void *pt_status;
	int pt_ret;
	if ((pt_ret = pthread_join((pthread_t)threadid,&pt_status)) == 0) {
		*status = (int)((long)pt_status);
	}
	// return (pt_ret == 0) ? QURT_EOK : QURT_ENOTHREAD;
	return pt_ret;
}

/*
 * This is unnecessarily complex maybe?
 * In POSIX it is only allowed for one thread to join. So we could eliminate
 * the reference counts.
 */
 /*
 * EJP: FIXME: need to possibly free our own stack.  That's an interesting trick,
 * I'm not entirely sure how we would do that.  I think the right answer is to:
 * have a small static stack
 * acquire a mutex for it
 * Switch to that stack
 * Free the thread stack
 * Have an atomic function that can release the mutex and trap to h2 thread stop.
 */
void qurt_thread_exit(int status)
{
	pthread_exit((void *)status);
}

void qurt_thread_get_name(char *name, unsigned char max_len)
{
	struct QURT_ugp_ptr *tmp;
	tmp = find_thread(h2_thread_myid());
	if (tmp) strlcpy(name,tmp->utcb.attr.name,max_len);
	else strlcpy(name,"NOTFOUND?!",max_len);
}

extern void qurt_qdi_local_client_init();

static void do_qurt_init()
{
	qurt_printf("Hello, Qurt!\n");
	h2_handle_errors(0);
	if (NULL == find_thread(h2_thread_myid())) qurt_thread_mainthread_become();
	qurt_memory_init();
	qurt_timer_init();
	qurt_qdi_local_client_init();
	qurt_exception_init();
}

/* Generic hook for all stuff that needs to be initialized */
__attribute__((constructor(1)))
void qurt_init()
{
	/* FIXME: pthread_once */
	static pthread_once_t qurt_init_once = PTHREAD_ONCE_INIT;
	pthread_once(&qurt_init_once,do_qurt_init);
}

