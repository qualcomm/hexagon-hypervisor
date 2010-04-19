/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <mqueue.h>
#include "mq_internal.h"

/* global array of mq's in a process */
mq * mqlist[MAX_MQ_IN_SYSTEM];
int max_mq_in_system = MAX_MQ_IN_SYSTEM;

/* index to the global array of mq's in a process */
unsigned int mqlist_next_slot = 0;

/* mutex to protect read/write on the global array of mq's */
pthread_mutex_t mqlist_mutex = PTHREAD_MUTEX_INITIALIZER;

#ifdef BLAST_LOCKFREE_LIFO

extern void * qbuf_alloc_real( void *buf_head );
extern void qbuf_free_real( void *buf_head, void * buf );

#define posix_lifo_pop qbuf_alloc_real
#define posix_lifo_push  qbuf_free_real

#endif

/* internal function to allocate and initialize memory used for a mq */
static int msgbuf_alloc_and_init(mq * mq_node, const struct mq_attr *attr)
{
    msg_node *free_msg;
    int      msg_buf_size = attr->mq_maxmsg * (attr->mq_msgsize + sizeof(msg_node));
    int      i;

    mq_node->buff_depot = malloc(msg_buf_size);
    if (!mq_node->buff_depot)
    {
        errno = ENOMEM;
        return -1;
    }
    
#ifndef BLAST_LOCKFREE_LIFO
    mq_node->free_list = mq_node->buff_depot; /* point put ptr to first free node */
    mq_node->recv_list = NULL;              /* point get ptr to NULL, since no msg available at init*/

    /* initialize the msgq msg buffer */
    free_msg = mq_node->free_list;
    for (i = 0; i < attr->mq_maxmsg; i++)
    {
        /* point to the location where msg is stored */
        free_msg->msg = (void *) ((unsigned int) free_msg + sizeof(msg_node));

        /* set up the next ptrs */
        if (i != attr->mq_maxmsg - 1)
            free_msg->next = (msg_node *) ((unsigned int) free_msg +
                                           sizeof(msg_node) + attr->mq_msgsize);
        else
            free_msg->next = NULL;

        free_msg = free_msg->next;
    }
#else
    mq_node->free_list = NULL; /* point put ptr to first free node */
    mq_node->recv_list = NULL;              /* point get ptr to NULL, since no msg available at init*/

    /* initialize the msgq msg buffer */
    free_msg = mq_node->buff_depot;
    for (i = 0; i < attr->mq_maxmsg; i++)
    {
        /* point to the location where msg is stored */
        free_msg->msg = (void *) ((unsigned int) free_msg + sizeof(msg_node));
        posix_lifo_push(&(mq_node->free_list), free_msg);        
        free_msg = (unsigned int)free_msg + (sizeof(msg_node) + attr->mq_msgsize);
    }
#endif        

    return 0;
}

/* internal function to free memory used for a mq */
static void msgbuf_free(mq * mq_node)
{
    if (mq_node->buff_depot)
        free(mq_node->buff_depot);
}

/* internal function to allocate and initialize memory used for threads waiting on a msgq */
static int threadbuf_alloc_and_init(mq * mq_node)
{
    int         i;
    thread_node * free_thread;
    int         thread_buf_size = MQ_MAX_NUM_WAITING_THREADS * sizeof(thread_node);

    mq_node->waitlist_buf= malloc(thread_buf_size);
    if (!mq_node->waitlist_buf)
    {
        errno = ENOMEM;
        return -1;
    }
    mq_node->empty_list = mq_node->waitlist_buf; /* point put ptr to first free node */
    mq_node->wait_list  = NULL;               /* point get ptr to NULL, since no thread waiting at init*/
    mq_node->thread_num = 0;                  /* No threads waiting */

    free_thread = mq_node->empty_list;
    for (i = 0; i < MQ_MAX_NUM_WAITING_THREADS; i++)
    {
        /* set up the next ptrs */
        if (i != MQ_MAX_NUM_WAITING_THREADS - 1)
            free_thread->next = (thread_node *) (( unsigned int ) free_thread +
                                                 sizeof(thread_node));
        else
            free_thread->next = NULL;

        free_thread = free_thread->next;
    }

    return 0;
}

/* internal function to free memory used for threads waiting on mq */
static void threadbuf_free(mq * mq_node)
{
    if (mq_node->waitlist_buf)
        free(mq_node->waitlist_buf);
}

/* free allocated memory for a mq */
static void mqlist_node_free(mq * mq_node)
{
    if (mq_node)
    {
        (void) pthread_mutex_destroy(mq_node->mq_lock);
        free(mq_node->mq_lock);
        threadbuf_free(mq_node);
        msgbuf_free(mq_node);
        free(mq_node);
    }
}

/* allocate node in the global mqlist */
mqd_t mqlist_node_alloc(const char *name, unsigned long mode, const struct mq_attr *attr)
{
    /* allocate memory for a mq */
    mq * mq_node = (mq *) malloc(sizeof(mq) + strlen(name) + 1);
    if (!mq_node)
    {
        errno = ENOMEM;
        goto mqlist_node_alloc_err;
    }

    /* allocate and init memory for msgs in the mq */
    if (0 != msgbuf_alloc_and_init(mq_node, attr))
        goto mqlist_node_alloc_err;

    /* allocate and init memory for threads waiting in the mq*/
    if (0 != threadbuf_alloc_and_init(mq_node))
        goto mqlist_node_alloc_err;

    /* initialize mq node */
    mq_node->mq_name = (char *) ((unsigned int) mq_node + sizeof(mq));
    (void) memcpy(mq_node->mq_name, name, strlen(name) + 1);
    mq_node->mq_lock            = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mq_node->mq_lock, 0);
    mq_node->mq_mode            = mode;
    mq_node->mq_refcount        = 1;
    mq_node->mq_attr.mq_flags   = attr->mq_flags;
    mq_node->mq_attr.mq_maxmsg  = attr->mq_maxmsg;
    mq_node->mq_attr.mq_msgsize = attr->mq_msgsize;
    mq_node->mq_attr.mq_curmsgs = 0;

    /* lock on global mqlist */
    (void) pthread_mutex_lock(&mqlist_mutex);

    /* system has more than MAX_MQ_IN_SYSTEM - FIRST_MQ_DESC mq's ? */
    if (MAX_MQ_IN_SYSTEM <= mqlist_next_slot)
    {
        (void) pthread_mutex_unlock(&mqlist_mutex);
        goto mqlist_node_alloc_err;
    }

    /* this should be true only ** ONCE ** in the system */
    if (0 == mqlist_next_slot)
    {
        memset(mqlist, 0, MAX_MQ_IN_SYSTEM);
        mqlist_next_slot = FIRST_MQ_DESC; /* the first descriptor */
    }

    mq_node->mq_desc           = (mqd_t) mqlist_next_slot;
    mqlist[mqlist_next_slot++] = mq_node;

    /* since an available mq node slot was consumed,
       set the next available slot in mqlist */
    while ((MAX_MQ_IN_SYSTEM > mqlist_next_slot) && mqlist[mqlist_next_slot])
        mqlist_next_slot++;

    /* unlock on global mqlist */
    (void) pthread_mutex_unlock(&mqlist_mutex);

    return mq_node->mq_desc;

 mqlist_node_alloc_err:

    mqlist_node_free(mq_node);
    return (mqd_t) -1;
}

/* destroy a mq and free its resources */
int mqlist_node_delete(const char * name)
{
    unsigned int slot      = FIRST_MQ_DESC;
    int          ret       = 0;
    mq           *mq_node  = NULL;

    (void) pthread_mutex_lock(&mqlist_mutex);

    while (slot < MAX_MQ_IN_SYSTEM)
    {
        if (mqlist[slot] && (0 == strcmp(mqlist[slot]->mq_name, name)))
        {
            mq_node = mqlist[slot];
            break;
        }

        slot++;
    }

    if (mq_node) /* found the msgq to be deleted */
    {
        /* refcount of a mq is alway accessed protected by the global array mutex */
        if (!mq_node->mq_refcount)
        {
            /* reclaim the slot and set next available slot in global mq list array */
            mqlist[mq_node->mq_desc] = 0;
            if (mq_node->mq_desc < (mqd_t) mqlist_next_slot)
                mqlist_next_slot = mq_node->mq_desc;

            /* delete the mq */
            mqlist_node_free(mq_node);
        }
    }
    else
    {
        /* mq requested doesnt exist in the process */
        errno = ENOENT;
        ret   = -1;
    }

    (void) pthread_mutex_unlock(&mqlist_mutex);

    return ret;
}

/* insert thread into waiting list of a mq
   note: this function is expected to be called with the mq node mutex locked
 */
static void mqlist_insert_thread(mq * mq_node, pthread_t thread, unsigned int prio)
{
    thread_node *p_curr, *p_prev, *p_node;

    /* do not check NULL pointer since it is internal function */

    if (mq_node->thread_num >= MQ_MAX_NUM_WAITING_THREADS)
    {
        assert(0);
        return;
    }

    p_prev = NULL;
    p_curr = mq_node->wait_list;

    while (p_curr && (p_curr->prio > prio))
    {
        p_prev = p_curr;
        p_curr = p_curr->next;
    }

    if(p_curr && p_curr->thread == thread)
    {
        /* already in the list */
        return;
    }

    p_node = mq_node->empty_list;
    if (NULL == p_node)
	{
        assert(0);
        return;
	}
    
    mq_node->empty_list = mq_node->empty_list->next;

    p_node->next   = NULL;
    p_node->thread = thread;
    p_node->prio   = prio;

    if (!p_prev)
        mq_node->wait_list = p_node;
    else
        p_prev->next = p_node;

    p_node->next = p_curr;
    ++mq_node->thread_num;
    
    return;
}

/* removes specified thread from the waiting list of a mq
   if no thread is specified, the first thread on the waiting list is removed
   note: this function is expected to be called with the mq node mutex locked
 */
void mqlist_remove_thread(mq * mq_node, pthread_t thread,
                          pthread_t *out_thread, unsigned int * out_prio)
{
    thread_node *p_curr, *p_prev;

    p_prev = NULL;
    p_curr = mq_node->wait_list;
    while (p_curr)
    {
        /* !thread takes care of the case when the very first thread on the
           waiting list needs to be removed
         */
        if (!thread || (p_curr->thread == thread))
            break;

        p_prev = p_curr;
        p_curr = p_curr->next;
    }

    if (p_curr)
    {
        if (out_thread)
            *out_thread = p_curr->thread;
        if (out_prio)
            *out_prio = p_curr->prio;

        if (!p_prev)
            mq_node->wait_list = p_curr->next;
        else
            p_prev->next = p_curr->next;

        p_curr->next        = mq_node->empty_list;
        mq_node->empty_list = p_curr;

        mq_node->thread_num--;
    }

    return;
}

/* put msg in the mq and remove the first waiting thread if there is one */
int mqlist_msg_put(mqd_t mq_desc, const char * msg, size_t msg_len, unsigned int msg_prio,
                   pthread_t * thread, unsigned int * prio)
{
    msg_node *cur_node;
    msg_node *pre_node = NULL;
    msg_node *new_node;
    mq       *mq_node;
    int      ret       = -1;

    /* do not check NULL pointer since it is internal function */

    mq_node = mqlist[mq_desc];
    if (!mq_node)
        return -1;

    *thread = 0;
    *prio   = 0;

    (void) pthread_mutex_lock(mq_node->mq_lock);

#ifndef BLAST_LOCKFREE_LIFO

    /*check if mq is full */
    if (mq_node->mq_attr.mq_maxmsg == mq_node->mq_attr.mq_curmsgs)
    {
        /* may need to implement blocking in case mq is full
           not sure if its ideal for real-time systems where
           msg put can end up getting called from IST/IRQ context */
        errno = EAGAIN;
        goto return_msg_put;
    }

    /* create and intialize the new msg node */
    new_node           = mq_node->free_list;
    mq_node->free_list = mq_node->free_list->next;
    new_node->next     = NULL;

#else
    new_node = posix_lifo_pop(&(mq_node->free_list));

    if (!new_node)
    {
        /* may need to implement blocking in case mq is full
           not sure if its ideal for real-time systems where
           msg put can end up getting called from IST/IRQ context */
        errno = EAGAIN;
        goto return_msg_put;
    }
#endif
    cur_node = mq_node->recv_list;
    while (cur_node)
    {
        if (cur_node->msg_prio < msg_prio)
            break;

        pre_node = cur_node;
        cur_node = cur_node->next;
    }

    if (pre_node)
        pre_node->next = new_node;
    else
        mq_node->recv_list = new_node;

    new_node->next = cur_node;

    /* copy the msg. truncate the msg to max msgsize incase msg provided is larger */
    /* note: we can choose to fail if msg sent is more than max size specified */
    memcpy((void *) new_node->msg, (void *) msg, ((mq_node->mq_attr.mq_msgsize > (long) msg_len) ?
                                                  (long) msg_len : mq_node->mq_attr.mq_msgsize));
    new_node->msg_prio = msg_prio;
    new_node->msg_len  = msg_len;

    /* increment number of msgs currently in the mq */
    mq_node->mq_attr.mq_curmsgs++;

    /* now remove the first waiting thread if there was one */
    mqlist_remove_thread(mq_node, 0, thread, prio);

    ret = 0;

 return_msg_put:
    (void) pthread_mutex_unlock(mq_node->mq_lock);
    return ret;
}

/* get the first msg available on mq.
   if msg not available and thread specified, put the thread in waitlist for notification
   when msg is available
 */
int mqlist_msg_get(mqd_t mq_desc, char * msg, size_t msg_len, unsigned int * msg_prio,
                   size_t * msg_received, pthread_t thread, unsigned int prio)
{
    msg_node *pickup_node;
    mq       *mq_node;
    size_t   copy_bytes;
    int      ret = -1;

    /* do not check NULL pointer since it is internal function */

    mq_node = mqlist[mq_desc];
    if (!mq_node)
        return -1;

    (void) pthread_mutex_lock(mq_node->mq_lock);

    /* no msgs in the mq ?*/
    if (!mq_node->mq_attr.mq_curmsgs)
    {
        /* if thread specified, put the thread in the waiting list */
        if (thread)
            mqlist_insert_thread(mq_node, thread, prio);

        errno = ETIMEDOUT; /* may need to implement appropriate underlying behaviour */
        goto return_msg_get;
    }

    pickup_node = mq_node->recv_list;
    if (pickup_node)
    {
        /* decrement the number of msgs currently available as we consume one here */
        mq_node->mq_attr.mq_curmsgs--;

        /* limit number of bytes to be copied into user passed buffer to max size of user passed buffer */
        copy_bytes = (msg_len < pickup_node->msg_len ? msg_len : pickup_node->msg_len);

        /* copy msg into the user specified buffer */
        memcpy((void *) msg, (void *) pickup_node->msg, copy_bytes);

        /* return copied msg size in the out param */
        *msg_received = copy_bytes;

        /* return msg priority if out param specified */
        if (msg_prio)
            *msg_prio = pickup_node->msg_prio;

        /* return back consumed msg to the free list */
        pickup_node->msg_len  = 0;
        pickup_node->msg_prio = 0;

        mq_node->recv_list  = mq_node->recv_list->next;

#ifndef BLAST_LOCKFREE_LIFO

        pickup_node->next = mq_node->free_list;
        mq_node->free_list  = pickup_node;
#else
        posix_lifo_push(&(mq_node->free_list), pickup_node);
#endif
        ret               = 0;
    }

 return_msg_get:
    (void) pthread_mutex_unlock(mq_node->mq_lock);

    return ret;
}

/* check but not consume/return if a msg is available in the mq
   if thread & priority is specified, add the thread in the waitlist if msg is not
   available
 */
int mqlist_msg_exists(mqd_t mq_desc, pthread_t thread, unsigned int prio)
{
    mq  *mq_node;
    int ret = FALSE;

    /* do not check NULL pointer since it is internal function */

    mq_node = mqlist[mq_desc];
    if (!mq_node)
        return ret;

    (void) pthread_mutex_lock(mq_node->mq_lock);

    /* is msg available ?*/
    if (!mq_node->mq_attr.mq_curmsgs)
    {
        /* if thread specified, add it to the waitlist */
        if (thread)
            mqlist_insert_thread(mq_node, thread, prio);
    }
    else
        ret = TRUE;

    (void) pthread_mutex_unlock(mq_node->mq_lock);

    return ret;
}

/* search a mq by name */
mqd_t mqlist_node_search(const char * name)
{
    unsigned int slot    = FIRST_MQ_DESC;
    mqd_t        mq_desc = -1;

    (void) pthread_mutex_lock(&mqlist_mutex);

    while (MAX_MQ_IN_SYSTEM > slot)
    {
        if (mqlist[slot] && (0 == strcmp(mqlist[slot]->mq_name, name)))
        {
            mq_desc = slot;
            break;
        }

        slot++;
    }

    (void) pthread_mutex_unlock(&mqlist_mutex);

    return mq_desc;
}

/* increment ref count of a mq
   Note: this has to happen while locking the global mqlist array mutex
 */
int mqlist_node_incref(mqd_t mq_desc)
{
    int ref_count = -1;
    mq  *mq_node;

    if (!IS_MQ_DESC_VALID(mq_desc))
    {
        errno = EBADF;
        return -1;
    }

    (void) pthread_mutex_lock(&mqlist_mutex);

    mq_node = mqlist[mq_desc];
    if (mq_node)
        ref_count = ++mq_node->mq_refcount;

    (void) pthread_mutex_unlock(&mqlist_mutex);

    return ref_count;
}

/* decrement ref count of a mq
   Note: this has to happen while locking the global mqlist array mutex
 */
int mqlist_node_decref(mqd_t mq_desc)
{
    int ref_count = -1;
    mq  *mq_node;

    if (!IS_MQ_DESC_VALID(mq_desc))
    {
        errno = EBADF;
        return -1;
    }

    (void) pthread_mutex_lock(&mqlist_mutex);

    mq_node = mqlist[mq_desc];
    if (mq_node && mq_node->mq_refcount)
    {
        ref_count = --mq_node->mq_refcount;

        (void) pthread_mutex_lock(mq_node->mq_lock);;

        /* if the caller is in the mq's wait list, remove it */
        (void) mqlist_remove_thread(mq_node, pthread_self(), 0, 0);

        (void) pthread_mutex_unlock(mq_node->mq_lock);
    }

    (void) pthread_mutex_unlock(&mqlist_mutex);

    return ref_count;
}
