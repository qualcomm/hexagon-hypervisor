/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast.h>
#include <h2.h>
#include <blast_tls.h>
#include <blast_thread.h>

#include <string.h>
#include <stdio.h>

static struct BLAST_ugp_ptr *blast_root = NULL;
static h2_mutex_t blast_root_mutex;

#if 0
#define DEBUG_PRINTF printf
#else
#define DEBUG_PRINTF(...) /* nothing */
#endif

static struct BLAST_ugp_ptr *find_thread(int threadid)
{
	struct BLAST_ugp_ptr *tmp;
	h2_mutex_lock(&blast_root_mutex);
	tmp = blast_root;
	while ((tmp != NULL) && (tmp->utcb.thread_id != threadid)) tmp = tmp->next;
	h2_mutex_unlock(&blast_root_mutex);
	return tmp;
}

static void remove_thread(struct BLAST_ugp_ptr *ptr)
{
	struct BLAST_ugp_ptr *tmp;
	h2_mutex_lock(&blast_root_mutex);
	tmp = blast_root;
	if (ptr == tmp) blast_root = ptr->next;
	else while (tmp->next && (tmp->next != ptr)) tmp = tmp->next;
	if (tmp->next) tmp->next = tmp->next->next;
	free(ptr);
	h2_mutex_unlock(&blast_root_mutex);
}

static void add_thread(struct BLAST_ugp_ptr *ptr)
{
	h2_mutex_lock(&blast_root_mutex);
	ptr->next = blast_root;
	blast_root = ptr;
	h2_mutex_unlock(&blast_root_mutex);
	DEBUG_PRINTF("Added thread %x\n",h2_thread_myid());
}

/* mostly from blast_thread.c */
int blast_thread_initial_setup (struct BLAST_ugp_ptr *pUgp)
{
   /* Set thread_id */
   pUgp->utcb.thread_id = h2_thread_myid();

   /* Set UGP Value.  Need to do this after the call to h2_thread_myid above so
      that ugp == 0 will cause the thread_id trap to be called */
  __asm__ __volatile__ (
			"ugp = %0\n"
			:
			:"r"(pUgp)
			);

  add_thread(pUgp);

  /* Initialize the anysignal */
  blast_anysignal_init (&pUgp->utcb.anysignal);
  
  return 0;
}

/* from blast_thread.c */
void blast_trampoline (void *arg)
{
   struct BLAST_ugp_ptr *pUgp;

   pUgp = (struct BLAST_ugp_ptr *)arg;

   /* Initialize UGP and UTCB variables */
   blast_thread_initial_setup (pUgp);

   /* Enter thread entry */
   pUgp->utcb.entrypoint (pUgp->utcb.arg);
   blast_thread_exit(0);
}

/* FIXME: hackage */
int blast_thread_create(void *pc, void *stack, void *arg, 
	unsigned int prio, unsigned int asid, unsigned int hw_bitmask)
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

	struct BLAST_ugp_ptr *pUgp;

	/* from blast_thread.c */
	pUgp = (struct BLAST_ugp_ptr *)blast_malloc (sizeof (struct BLAST_ugp_ptr));
	assert(pUgp);
	memset (pUgp, 0, sizeof (struct BLAST_ugp_ptr));
	/* That initializes join_lock, join_cond, join_refcount */
	pUgp->utcb.entrypoint = pc;
	pUgp->utcb.arg = arg;
	pUgp->utcb.stack_ptr = (unsigned int *)stack;

	//	return h2_thread_create(pc,stack,arg,prio);
	return h2_thread_create((void *)blast_trampoline, stack, (void *)pUgp, prio);
}

int blast_thread_join(int threadid, int *status)
{
	*status = 0;
	int tmp;
	struct BLAST_ugp_ptr *ptr;
	ptr = find_thread(threadid);
	DEBUG_PRINTF("BLAST JOIN: found ptr %x\n",ptr);
	if (ptr == NULL) return 0;
	h2_mutex_lock(&ptr->join_lock);
	if (ptr->join_state != BLAST_JOIN_STATE_RUNNING) {
		DEBUG_PRINTF("BLAST JOIN: thread not running\n");
		return 0;
	}
	ptr->join_refcount++;
	while (ptr->join_state == BLAST_JOIN_STATE_RUNNING) {
		DEBUG_PRINTF("BLAST_JOIN: Waiting for thread %x to finish...\n",threadid);
		h2_cond_wait(&ptr->join_cond,&ptr->join_lock);
	}
	ptr->join_refcount--;
	tmp = ptr->join_refcount;
	h2_mutex_unlock(&ptr->join_lock);
	if (tmp == 0) {
		remove_thread(ptr);
	}
	return 0;
}

void blast_thread_exit(int status)
{
	struct BLAST_ugp_ptr *tmp;
	tmp = find_thread(h2_thread_myid());
	if (tmp) {
		h2_mutex_lock(&tmp->join_lock);
		tmp->join_state = BLAST_JOIN_STATE_DONE;
		if (tmp->join_refcount) {
			DEBUG_PRINTF("BLAST THREAD: %x waking up waiting threads...\n",h2_thread_myid());
			h2_cond_broadcast(&tmp->join_cond);
			h2_mutex_unlock(&tmp->join_lock);
		} else {
			DEBUG_PRINTF("BLAST THREAD: %x sees no waiters...\n",h2_thread_myid());
			h2_mutex_unlock(&tmp->join_lock);
			remove_thread(tmp);
		}
	}
	h2_thread_stop(status);
}

