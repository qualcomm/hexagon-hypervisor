/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _PTHREAD_INTERNAL_H_
#define _PTHREAD_INTERNAL_H_

#include <blast.h>
#include <blast_tls.h>
#include <blast_fd.h>
#include <errno.h>
#include <signal.h>
#include <mqueue.h>
#include "time_internal.h"

typedef blast_fd_table_t pthread_id_table_t;

extern pthread_id_table_t pthread_id_table;

#define pthread_id_table_init blast_fd_table_init_with_mem

#define pthread_id_new(handle) \
    blast_fd_table_add(&(pthread_id_table), (handle))

#define pthread_id_delete(obj) \
    blast_fd_table_remove(&(pthread_id_table), (blast_obj_t)(obj))

#define pthread_id_is_valid(obj) \
    blast_fd_table_is_valid_obj(&(pthread_id_table),(obj))

#define pthread_id_get_handle(m_obj) \
    blast_fd_table_get_handle(&(pthread_id_table), (m_obj))

/* BLAST Signal bit used for POSIX */
#define POSIX_SIGNAL_KERNEL_SIGNO      1
#define POSIX_MQ_NOTIF_KERNEL_SIGNO    2
#define POSIX_SIGALARM_KERNEL_SIGNO    3

/* Support up to MAX_TIMER_NUM timers per thread */
#define POSIX_MAX_TIMER_NUM            6

/* BLAST signo 3-8 are used for timer */
#define POSIX_TIMER_SIGNO_MIN          POSIX_SIGALARM_KERNEL_SIGNO
#define POSIX_TIMER_SIGNO_MAX          POSIX_SIGALARM_KERNEL_SIGNO + POSIX_MAX_TIMER_NUM - 1

#define POSIX_SIGNAL_MASK              1 << (POSIX_SIGNAL_KERNEL_SIGNO - 1)
#define POSIX_MQ_NOTIF_MASK            1 << (POSIX_MQ_NOTIF_KERNEL_SIGNO - 1)
#define POSIX_SIGALARM_MASK            1 << (POSIX_SIGALARM_KERNEL_SIGNO - 1)
#define POSIX_TIMER_SIGNO_MIN_MASK     1 << (POSIX_TIMER_SIGNO_MIN - 1)
#define POSIX_TIMER_SIGNO_MAX_MASK     1 << (POSIX_TIMER_SIGNO_MAX - 1)
#define POSIX_TIMER_MASK               0x00FC /* 11111100 */

extern int pthread_tcb_key;

typedef void* (*start_func)(void*);
typedef struct pthread_i   pthread_i;
struct pthread_i
{
    unsigned long     magic;
    pthread_t         pthread;
    int               blastid;

    //sigset_t  sigmask;    /* Blocked signals */
    sigset_t          sigpending; /* Pending mask */
    sigset_t          sigwaiting; /* Waiting mask */

    start_func        start_routine;
    void              *args;

    pthread_attr_t    attr;
    blast_anysignal_t sigs;
    void              * sigaction_ptrs[SIGRTMAX];

    timer_i           *timers[POSIX_MAX_TIMER_NUM]; /* FIXME: dynamic malloc buffer to save memory ? */
    int               last_err;
    int               exitstatus;
    blast_sem_t       join_lock;

    fd_set            *select_mask;
    //FIXME: add mutex in this struct
};

/* Functions for internal use only */
//pthread_i* _init_ltcb(pthread_t thread);
int  _getltcb(pthread_i **ltcb, pthread_t thr);
void _deinit_ltcb(pthread_t thr);
int *_geterrnoaddr(void);
int  _getltcb_self(pthread_i **ltcb);

#endif /* _PTHREAD_INTERNAL_H_ */
