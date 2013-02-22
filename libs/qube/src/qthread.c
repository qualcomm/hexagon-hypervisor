/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast.h>
#include <hexagon_protos.h>
#include <qube.h>
//#include "blast_thread.h"
//#include "blast_tls.h"
//#include "blast_power.h"

struct qthread_emu_struct {
	struct qthread_emu_struct *next;
	qthread_t id;
	int blastid;
	qthread_attr_t attrs;
	void (*realfunc)(void *);
	void *arg;
	uint32_t int_register;
	int exitstatus;
    int  tid_reg;
    blast_anysignal_t int_signal;
    unsigned int signal_mask;
    unsigned int signal_to_ack;
    unsigned short signal_to_int[32];
};

static struct qthread_emu_struct *head = NULL;

//static blast_mutex_t thmutex = {0, QTHREADMUTEX_QUEUE_ID,0,0};
//static blast_mutex_t thmutex = {MUTEX_MAGIC, 0,0,0};
static h2_mutex_t thmutex = H2_MUTEX_T_INIT;
static int unique_ids = 0;
int qube_tls_key;

static inline struct qthread_emu_struct *get_myutcb (void)
{
	struct qthread_emu_struct *tmp;

	tmp = (struct qthread_emu_struct *)blast_tls_getspecific (qube_tls_key);
	return tmp;
}

static inline struct qthread_emu_struct *get_qthread(qthread_t id)
{
	struct qthread_emu_struct *ret;
	h2_mutex_lock(&thmutex);
	for (ret = head; ret != NULL; ret = ret->next) {
		if (ret->id == id) break;
	}
	h2_mutex_unlock(&thmutex);
	return ret;
}

int qthread_get_blastid(qthread_t thr)
{
	struct qthread_emu_struct *tmp;
	int blastid;

	h2_mutex_lock(&thmutex);
	for (tmp = head; tmp != NULL; tmp = tmp->next) {
		if (tmp->id == thr) break;
	}

	if (tmp == NULL) {
		h2_mutex_unlock(&thmutex);
		return EINVALID;
	}

	blastid = tmp->blastid;
	h2_mutex_unlock(&thmutex);
	return blastid;
}

int qthread_create(qthread_t *thr, qthread_attr_t *attr)
{
	struct qthread_emu_struct *tmp;
	if (thr == NULL) return EVAL;
	h2_mutex_lock(&thmutex);
	if ((tmp = blast_malloc(sizeof(*tmp))) == NULL) {
		h2_mutex_unlock(&thmutex);
		return EMEM;
	}
	if (attr) {
		memcpy(&tmp->attrs,attr,sizeof(*attr));
	}
	else {
        	qthread_attr_init(&tmp->attrs);
	}
	*thr = tmp->id = ++unique_ids;
	tmp->int_register = 0;
	tmp->next = head;
    tmp->tid_reg = tmp->attrs.tid;
        /* interrupt tls */
        blast_anysignal_init(&tmp->int_signal);
        tmp->signal_mask = 0;
        tmp->signal_to_ack = 0xff;
        memset(&tmp->signal_to_int, 0, 64);
	head = tmp;
	h2_mutex_unlock(&thmutex);
	return EOK;
}

static void trampoline(void *argv)
{
	struct qthread_emu_struct *arg = argv;
	int rc;

    blast_thread_set_tid(arg->tid_reg);
    unsigned long long name0; 
    unsigned long long name1;
    memcpy( &name0, arg->attrs.name, 8);
    memcpy( &name1, &(arg->attrs.name[8]), 8);
    qthread_set_name(name0, name1);
	/* Set up the TLS */
	rc = blast_tls_setspecific (qube_tls_key, (const void *)arg);
	if (rc != 0) {
		// !!! PANIC
		exit (0);
	}
	arg->realfunc(arg->arg);
	qthread_exit(0);
}

int qthread_start(qthread_t thr, void (*start_func)(void *), void *arg)
{
	struct qthread_emu_struct *tmp;
	int prio;

	if (start_func == NULL) return EVAL;
	tmp = get_qthread(thr);
	if (tmp == NULL) return EINVALID;

	if (tmp->attrs.stackaddr == 0) {
		if ((tmp->attrs.stackaddr = (blast_addr_t)(blast_malloc(tmp->attrs.stacksize))) == (unsigned int)NULL) {
			return EMEM;
		}
	}
	tmp->arg = arg;
	tmp->realfunc = start_func;
	prio = tmp->attrs.priority;
#ifdef BLAST_STATIC
	if (prio > 31) prio = 31;
	if (prio < 0) prio = 0;
	prio = 31-prio;
#else //#ifdef BLAST_STATIC
    prio = 255-prio;
#endif //#ifdef BLAST_STATIC

	tmp->blastid = blast_thread_create(trampoline,(void *)((tmp->attrs.stackaddr+tmp->attrs.stacksize) & -8),tmp,prio,/* ASID? */ 0, tmp->attrs.hw_bitmask);
	//blast_printf("ret: %d\n",tmp->blastid);
	if (tmp->blastid == -1) return EINVALID;
	return EOK;
}

qthread_t qthread_myself(void)
{
	struct qthread_emu_struct *tmp;

	tmp = get_myutcb ();

	return tmp->id;
}

int qthread_get_attr(qthread_t thr, qthread_attr_t *attr)
{
	struct qthread_emu_struct *tmp;

	if (attr == NULL) return EVAL;
	if ((tmp = get_qthread(thr)) == NULL) return EINVALID;

	memcpy(attr,&tmp->attrs,sizeof(qthread_attr_t));
	return EOK;
}

void qthread_exit(int status)
{
	struct qthread_emu_struct *tmp;
	struct qthread_emu_struct **ptr;

	tmp = get_myutcb ();
	if (tmp == NULL) {
		blast_printf("Qube TLS not setup properly\n");
		// Still exit the thread
		blast_thread_exit (status);
		return;
	}

	h2_mutex_lock(&thmutex);
	/* Remove from the list */
	ptr = &head;
	while (*ptr != NULL && *ptr != tmp) {
		ptr = &(*ptr)->next;
	}
	if (*ptr != tmp) {
		blast_printf("Could not find my qube thread info\n");
		h2_mutex_unlock(&thmutex);
		// Still exit the thread
		blast_thread_exit (status);
		return;
	}
        *ptr = tmp->next;
	h2_mutex_unlock(&thmutex);
	blast_free (tmp);
	// exit the thread
	blast_thread_exit (status);
}

int qthread_join(qthread_t thr, int *status)
{
	int blastid, rc;

	if (status == NULL) return EVAL;

	blastid = qthread_get_blastid (thr);
	if (blastid == EINVALID) {
		return EINVALID;
	}

	rc = blast_thread_join (blastid, status);

	return rc;
}

/* If blast_init () is called from main (), the thread is unaware of qube */
int qthread_root_setup (void)
{
	struct qthread_emu_struct *tmp;

//    thmutex.queue = blast_futex_alloc_wait_queue();

	h2_mutex_lock(&thmutex);
	if ((tmp = blast_malloc(sizeof(*tmp))) == NULL) {
		h2_mutex_unlock(&thmutex);
		return EMEM;
	}
	memset (&tmp->attrs, 0, sizeof(qthread_attr_t));
	tmp->id = ++unique_ids;
	tmp->int_register = 0;
	tmp->next = head;
	head = tmp;
	h2_mutex_unlock(&thmutex);

	tmp->blastid = blast_thread_myid();

    unsigned long long name0=0, name1=0;    
    memcpy( &name0, "main", 4);
    qthread_set_name(name0, name1);
	/* Set up the TLS */
	blast_tls_setspecific (qube_tls_key, (const void *)tmp);

    //exit(main());

	return EOK;
}

int qinterrupt_register(int int_num)
{
	int i, next_slot = -1, status;
	struct qthread_emu_struct *tmp;

	//blast_mutex_lock(&thmutex);
	//blast_printf( "qinterrupt_register %d by %x\n", int_num, blast_thread_myid());
	//blast_mutex_unlock(&thmutex);

	tmp = get_myutcb ();
        if (tmp->signal_mask == 0x7fffffff)
            return EINVALID;

        for(i = 0; i < 31; i++)
        {
            if (tmp->signal_to_int[i] == int_num && (tmp->signal_mask & 1<<i))
            {
                /* registered before, reuse previous record */
                next_slot = i;
                break;
            }
            
            if (tmp->signal_to_int[i] == 0 && next_slot < 0)
                /* find next available signal */
                next_slot = i;
                
        }

        if (next_slot < 0) return EINVALID;

        blast_anysignal_clear(&tmp->int_signal, (0x80000000 | (1<< next_slot)));
        tmp->signal_mask |= 1<<next_slot;
        tmp->signal_to_int[next_slot] = int_num;

        if ((status = blast_register_interrupt(int_num, &tmp->int_signal, 1<<next_slot)) != EOK)
        {
            printf("isr thread failed to register interrupt");
            tmp->signal_mask &= ~(1<<next_slot);
            tmp->signal_to_int[next_slot] = 0;
            return EINVALID;
        }

	return EOK;
}

int qinterrupt_deregister(int int_num)
{

        blast_deregister_interrupt(int_num);
	return EOK;
}

int qinterrupt_receive (unsigned int *int_num)
{
    struct qthread_emu_struct *tmp;
    unsigned int ret_signal;
    tmp = get_myutcb ();
    if (tmp->signal_to_ack < 31)
    {
        blast_anysignal_clear(&tmp->int_signal, (1<<tmp->signal_to_ack));
        if (blast_ack_interrupt(tmp->signal_to_int[tmp->signal_to_ack]) == -1)
            return ENO_INTERRUPTS;
        tmp->signal_to_ack = 0;
    }
    ret_signal = blast_anysignal_wait(&tmp->int_signal, 0x80000000|tmp->signal_mask);
    if (!ret_signal) return ENO_INTERRUPTS;
    if (ret_signal & 0x80000000) return ENO_INTERRUPTS;
    tmp->signal_to_ack = 31 - Q6_R_cl0_R(ret_signal);
    *int_num = (unsigned int)tmp->signal_to_int[tmp->signal_to_ack];
    return EOK;
}

void qthread_wait_for_idle (void)
{
   blast_power_wait_for_idle ();

}

void qthread_wait_for_active (void)
{
   blast_power_wait_for_active ();

}
