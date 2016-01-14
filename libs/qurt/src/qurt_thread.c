/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt.h>
#include <h2.h>
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

unsigned int __thread dummy_1 = 1;
unsigned int __thread dummy_0;

#define MAX_TLS_DWORDS 32

extern char tdata_start;
extern char tdata_end;
extern char tbss_start;
extern char tbss_end;

static inline int size_of_tls()
{
	// qurt_printf("tdata=%p..%p tbss=%p..%p\n",&tdata_start,&tdata_end,&tbss_start,&tbss_end);
	return (&tbss_end - &tdata_start);
}

static inline void dup_tls(void *ugp)
{
	// qurt_printf("ugp=%p size_of_tls=%d\n",ugp,size_of_tls());
	unsigned char *tls_end_ptr = ugp;
	unsigned char *tls_start_ptr = tls_end_ptr - size_of_tls();
	memcpy(tls_start_ptr,&tdata_start,&tdata_end-&tdata_start);
}

static inline void qurt_tls_check()
{
	if (size_of_tls() > MAX_TLS_DWORDS) {
		qurt_printf("TLS size too big: %d > %d!\n",size_of_tls(),MAX_TLS_DWORDS);
	}
}

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
	struct QURT_ugp_ptr *tmp;
	unsigned char *alloc_ptr;
	h2_mutex_lock(&qurt_root_mutex);
	tmp = qurt_root;
	if (ptr == tmp) qurt_root = ptr->next;
	else while (tmp->next && (tmp->next != ptr)) tmp = tmp->next;
	if (tmp->next) tmp->next = tmp->next->next;
	h2_mutex_unlock(&qurt_root_mutex);
	alloc_ptr = (unsigned char *)ptr;
	alloc_ptr -= size_of_tls();
	free(alloc_ptr);
}

void qurt_thread_attr_init (qurt_thread_attr_t *attr)
{
    attr->name[0] = 0;
    attr->tcb_partition = QURT_THREAD_ATTR_TCB_PARTITION_DEFAULT;
    attr->priority = QURT_THREAD_ATTR_PRIORITY_DEFAULT;
    attr->asid = QURT_THREAD_ATTR_ASID_DEFAULT;
    attr->affinity = QURT_THREAD_ATTR_AFFINITY_DEFAULT;
    attr->bus_priority = QURT_THREAD_ATTR_BUS_PRIO_DEFAULT;
    attr->timetest_id = QURT_THREAD_ATTR_TIMETEST_ID_DEFAULT;
    attr->stack_size = 0;
    attr->stack_addr = 0;
}

static void add_thread(struct QURT_ugp_ptr *ptr)
{
	h2_mutex_lock(&qurt_root_mutex);
	ptr->next = qurt_root;
	qurt_root = ptr;
	h2_mutex_unlock(&qurt_root_mutex);
	DEBUG_PRINTF("Added thread %x\n",h2_thread_myid());
}

/* mostly from qurt_thread.c */
static inline void qurt_thread_initial_setup (struct QURT_ugp_ptr *pUgp)
{

	/* We now set the thread id in thread_create already, from the return value
		 of h2_thread_create */

	//   pUgp->utcb.thread_id = h2_thread_myid();

   /* Set UGP Value.  Need to do this after the call to h2_thread_myid above so
      that ugp == 0 will cause the thread_id trap to be called */
  __asm__ __volatile__ (
			"ugp = %0\n"
			:
			:"r"(pUgp)
			);

	//  add_thread(pUgp);
}

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

/* FIXME: hackage */
int qurt_thread_create(qurt_thread_t *thread_id, qurt_thread_attr_t *attr, void (*entrypoint) (void *), void *arg)
//int qurt_thread_create(void *pc, void *stack, void *arg, unsigned int prio, unsigned int asid, unsigned int hw_bitmask)
{
	/*
	 * H2's thread create prototype is:
	 * H2K_thread_create(u32_t pc, u32_t sp, u32_t arg, u32_t prio, u32_t trapmask,
	 *    H2K_thread_context *me)
	 * The trap automatically fills in the *me pointer, so it's really just that
	 * ASID thing that needs changing.  Just drop that on the floor.
	 * drop the hw_bitmask on the floor
	 * might as well fudge the priority too
	 */
	/* EJP: Also find TLS regions, allocate and duplicate them here. */
	unsigned char *alloc_tmp;
	struct QURT_ugp_ptr *pUgp;
	unsigned char *stackaddr;
	unsigned int stacksize;

	/* from qurt_thread.c */
	if ((alloc_tmp = qurt_calloc (1,size_of_tls() + sizeof (struct QURT_ugp_ptr))) == NULL) {
		return QURT_EFATAL;
	}
	pUgp = (struct QURT_ugp_ptr *)(alloc_tmp + size_of_tls());

	stackaddr = attr->stack_addr;
	stacksize = attr->stack_size;
	pUgp->utcb.entrypoint = (void *)entrypoint;
	pUgp->utcb.arg = arg;
	pUgp->utcb.attr = *attr;
	if (stackaddr == NULL) {
		stacksize = stacksize & -8;
		if ((stackaddr = malloc(stacksize)) == NULL) {
			free(alloc_tmp);
			return QURT_EFATAL; // or maybe ENOMEM? But not in ERRORS list
		};
		pUgp->stack_self_allocated = 1;
		pUgp->utcb.attr.stack_addr = stackaddr;
	}
	dup_tls(pUgp);
	/* That initializes join_lock, join_cond, join_refcount, join_done */

	*thread_id = h2_thread_create((void *)qurt_trampoline, (unsigned int *)(((unsigned int)stackaddr + stacksize) & (-8)), (void *)pUgp, attr->priority);

	if (-1 != *thread_id) {
		pUgp->utcb.thread_id = *thread_id;
		add_thread(pUgp);
	} else {
		if (pUgp->stack_self_allocated) free(stackaddr);
		free(alloc_tmp);
	}
	/* qurt_printf("QURT: created thread <%s> id=%x stack=%x entry=%x\n", */
	/* 	attr->name, */
	/* 	*thread_id, */
	/* 	((unsigned int)attr->stack_addr+attr->stack_size) & -8, */
	/* 	entrypoint); */
 	return (*thread_id == -1) ? QURT_EFATAL : QURT_EOK;
}

int blast_thread_create(void *pc, void *stack, void *arg, 
												unsigned int prio, unsigned int asid, unsigned int hw_bitmask) {

	qurt_thread_attr_t *attr;
	qurt_thread_t id = 0;

	if ((attr = (qurt_thread_attr_t *)qurt_malloc (sizeof (qurt_thread_attr_t))) == NULL) {
		return QURT_EFATAL;
	}
	memset (attr, 0, sizeof (qurt_thread_attr_t));
	attr->priority = prio;
	attr->stack_addr = stack;

	qurt_thread_create(&id, attr, pc, arg);
	return id;
}

/* Have a thread become a qurt thread: put it in all the data structures and allocate ugp stuff */
/*
 * EJP: note that we shouldn't malloc here, because we might call this before malloc is OK.
 * But... somehow we need to allocate storage for TLS and copy over the TLS data.
 */
void qurt_thread_mainthread_become()
{
	static struct {
		unsigned long long int tls_data_area[MAX_TLS_DWORDS];
		struct QURT_ugp_ptr main_ugp_storage;
	} x;
	memset (&x, 0, sizeof(x));
	struct QURT_ugp_ptr *pUgp = &x.main_ugp_storage;
	dup_tls(pUgp);
	pUgp->utcb.thread_id = h2_thread_myid();
	add_thread(pUgp);
	qurt_thread_initial_setup(pUgp);
}

int qurt_thread_join(unsigned int threadid, int *status)
{
	*status = 0;
	int tmp;
	struct QURT_ugp_ptr *ptr;
	ptr = find_thread(threadid);
	DEBUG_PRINTF("QURT JOIN: found ptr %x\n",ptr);
	if (ptr == NULL) return 0;
	h2_mutex_lock(&ptr->join_lock);
	ptr->join_refcount++;
	while (ptr->join_state == QURT_JOIN_STATE_RUNNING) {
		DEBUG_PRINTF("QURT_JOIN: Waiting for thread %x to finish...\n",threadid);
		h2_cond_wait(&ptr->join_cond,&ptr->join_lock);
	}
	ptr->join_refcount--;
	tmp = ptr->join_refcount;
	*status = ptr->status;
	h2_mutex_unlock(&ptr->join_lock);
	if (tmp == 0) {
		h2_sem_up(&ptr->join_done);
	}
	*status = ptr->status;
	return 0;
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
	struct QURT_ugp_ptr *tmp;
	tmp = find_thread(h2_thread_myid());
	/* EJP: FIXME: call TLS destructors */
	if (tmp) {
		h2_mutex_lock(&tmp->join_lock);
		tmp->join_state = QURT_JOIN_STATE_DONE;
		tmp->status = status;
		if (tmp->join_refcount) {
			DEBUG_PRINTF("QURT THREAD: %x waking up waiting threads...\n",h2_thread_myid());
			h2_cond_broadcast(&tmp->join_cond);
			h2_mutex_unlock(&tmp->join_lock);
		} else {
			DEBUG_PRINTF("QURT THREAD: %x sees no waiters...\n",h2_thread_myid());
			h2_mutex_unlock(&tmp->join_lock);
		}
	}
	h2_sem_down(&tmp->join_done);
	remove_thread(tmp);
	h2_thread_stop(0);
}

void qurt_thread_get_name(char *name, unsigned char max_len)
{
	struct QURT_ugp_ptr *tmp;
	tmp = find_thread(h2_thread_myid());
	if (tmp) strlcpy(name,tmp->utcb.attr.name,max_len);
	else strlcpy(name,"NOTFOUND?!",max_len);
}

extern int _posix_init();
extern void qurt_qdi_local_client_init();
/* Generic hook for all stuff that needs to be initialized */
void qurt_init()
{
	static int initted = 0;
	if (initted) return;
	qurt_printf("Hello, Qurt!\n");
	initted = 1;
	h2_handle_errors(0);
	qurt_memory_init();
	qurt_timer_init();
	qurt_qdi_local_client_init();
	(void)_posix_init();
	qurt_exception_init();
	qurt_tls_check();
}

