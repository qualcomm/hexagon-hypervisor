/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _MQ_INTERNAL_H_
#define _MQ_INTERNAL_H_

#include <mqueue.h>

#if 0
#define MSG_SIZE                      100
#define MAX_NO_MSG                    10
#else
#define FIRST_MQ_DESC                 4   /* mq descriptor values start from 4 */
#define MAX_MQ_IN_SYSTEM              256 /* should be equal to FD_SETSIZE. System will 
                                             support (256 - 4) mq's in a process */
#define MAX_MQ_PER_THREAD             32  /* the number of mq that each thread can pselect */
                                             
#define MAX_MSG_SIZE                  100 /* For LTE-RRC tests only, need to pass pointers for such big buffers */
#define MAX_NO_MSG                    10  /* default maximum number of msgs per mq */
#define MQ_MAX_NUM_WAITING_THREADS    10  /* default maximum number of threads that can wait on a mq */
#define IS_MQ_DESC_VALID(x)    ((x >= 4) && (x < MAX_MQ_IN_SYSTEM))
#endif

typedef struct _msg_node   msg_node;
struct _msg_node
{
    msg_node * next;
    int      msg_prio;
    size_t   msg_len;
    void     * msg;
};

typedef struct _thread_node   thread_node;
struct _thread_node
{
    int         prio;
    pthread_t   thread;
    thread_node * next;
};

typedef struct _mq   mq;
struct _mq
{
    char            *mq_name;      /* queue name */
    int             mq_refcount;   /* reference count */
    mqd_t           mq_desc;       /* mq descriptor */
    pthread_mutex_t *mq_lock;       /* lock to be used while updating mq node */
    unsigned long   mq_mode;       /* queue permission */
    struct mq_attr  mq_attr;       /* attribute */

    msg_node        *buff_depot;   /* list that holds msgs */
    msg_node        *free_list;    /* send msg is copied here */
    msg_node        *recv_list;    /* receive msg is picked up here */

    thread_node     *waitlist_buf; /* threads waiting on this msgq */
    thread_node     *empty_list;   /* thread gets added here */
    thread_node     *wait_list;    /* thread gets picked up here */
    int             thread_num;    /* number of threads waiting */

    //mq              *next;         /* point to next node */
};

/* mq internal apis */
mqd_t mqlist_node_alloc(const char *name, unsigned long mode, const struct mq_attr *attr);
mqd_t mqlist_node_search(const char * name);
int   mqlist_node_delete(const char * name);
int   mqlist_node_incref(mqd_t mq_desc);
int   mqlist_node_decref(mqd_t mq_desc);
int   mqlist_msg_exists(mqd_t mq_desc, pthread_t thread, unsigned int prio);
int   mqlist_msg_put(mqd_t mq_desc, const char * msg, size_t msg_len, unsigned int msg_prio,
                     pthread_t * thread, unsigned int * prio);
int   mqlist_msg_get(mqd_t mq_desc, char * msg, size_t msg_len, unsigned int * msg_prio,
                     size_t * msg_received, pthread_t thread, unsigned int prio);

void mqlist_remove_thread(mq * mq_node, pthread_t thread,
                          pthread_t *out_thread, unsigned int * out_prio);

#endif /* _MQ_INTERNAL_H_ */
