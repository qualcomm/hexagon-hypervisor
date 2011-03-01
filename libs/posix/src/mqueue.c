/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <sys/types.h>
#include <common/time.h>
#include <stdio.h>
#include <limits.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <q6protos.h> /* Q6_R_ct0_R */

#include <pthread.h>
#include <mqueue.h>
#include "pthread_internal.h"
#include "mq_internal.h"

/* debug variable */
unsigned int mq_fd_err_cnt = 0;

extern mq * mqlist[MAX_MQ_IN_SYSTEM];

/* check if name is a valid path */
static inline int is_valid_path(const char *name)
{
    return 0;
}

/* default attribute for mq_open() used when attr == NULL */
static struct mq_attr attr_default =
{
    0,               /* mq_flags   */
    MAX_NO_MSG,      /* mq_maxmsg  */
    MAX_MSG_SIZE,    /* mq_msgsize */
    0                /* mq_curmsgs */
};

/* API Functions */
mqd_t mq_open(const char *name, int oflag, ...)
{
    unsigned long  mode;
    struct mq_attr *attr;
    mqd_t          mq_desc = 0;
    va_list        ap;

    /*lint -save -e10 -e26 -e415 -e416 -e530 -e831*/
    va_start(ap, oflag);
    mode = va_arg(ap, unsigned long);
    attr = (struct mq_attr *) va_arg(ap, struct mq_attr *);
    va_end(ap);
    /*lint -restore*/

    if (is_valid_path(name) < 0)
        return (mqd_t) -1;

    /* check oflag arg */
    switch ((oflag & O_ACCMODE))
    {
    case O_RDONLY:
    case O_WRONLY:
    case O_RDWR:
        break;
    default:
        return (mqd_t) -1;    /* EACCES; */
    }

    /* search name in the mq descriptor table */
    mq_desc = mqlist_node_search(name);
    if (IS_MQ_DESC_VALID(mq_desc)) /* found */
    {
        /* return the found mq descriptor */
        if ((oflag & (O_CREAT | O_EXCL)) == (O_CREAT | O_EXCL))
        {
            errno = EEXIST;
            return (mqd_t) -1;
        }

        if (-1 == mqlist_node_incref(mq_desc))
            return (mqd_t) -1;

        return mq_desc; /* return existing mq */
    }

    /* not found an existing mq in the list */
    if (!(oflag & O_CREAT))
    {
        errno = ENOENT;
        return (mqd_t) -1;
    }

    /*
     * If attr is NULL, the mq is created with implemetation-defined
     * default mq attributes (POSIX).
     */
    if (NULL == attr)
        attr = &attr_default;

    /* if O_NONBLOCK is passed in, save it to mq attr */
    if (oflag & O_NONBLOCK)
    {
        attr->mq_flags |= O_NONBLOCK;
    }

    /* make sure mq buffer will be 4 bytes aligned */
    attr->mq_msgsize = (attr->mq_msgsize-1)/4*4+4;  

    /* create new mq */
    return mqlist_node_alloc(name, mode, attr);
}

int mq_close(mqd_t mq_desc)
{
    if (-1 != mqlist_node_decref(mq_desc))
        return 0;

    return -1;
}

int mq_unlink(const char *name)
{
    if (!name || is_valid_path(name) < 0)
    {
        errno = EBADF;
        return -1;
    }

    return mqlist_node_delete(name);
}

int mq_send(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
            unsigned int msg_prio)
{
    return mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, NULL);
}

int mq_timedsend(mqd_t mqdes, const char *msg_ptr, size_t msg_len,
                 unsigned int msg_prio,
                 const struct timespec *abs_timeout)
{
    pthread_t    thread      = 0;
    unsigned int thread_prio = 0;
    pthread_i    * ltcb;

    /* if abs_timeout is not NULL, only 0 timeout is supported */
    if ((NULL != abs_timeout) && (abs_timeout->tv_sec != 0 || abs_timeout->tv_nsec != 0))
    {
        errno = EINVAL;
        return -1;
    }

    if (0 != mqlist_msg_put(mqdes, msg_ptr, msg_len, msg_prio, &thread, &thread_prio))
        return -1;

    /* notify the thread already waiting on the msgq */
    if (thread)
    {
        if (thread_prio < PTHREAD_MIN_PRIORITY || thread_prio > PTHREAD_MAX_PRIORITY)
        {
            errno = EINVAL;
            return -1;
        }

        if (0 != _getltcb(&ltcb, thread))
        {
            errno = ESRCH;
            return -1;
        }

        /* set select_mask */
        if(ltcb->select_mask)
            FD_SET(mqdes, ltcb->select_mask);
        
        //FIXME: will check the return value of this function when it is clarified by BLAST team.
        blast_anysignal_set(&ltcb->sigs, POSIX_MQ_NOTIF_MASK);
    }

    return 0;
}

ssize_t mq_receive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                   unsigned int *msg_prio)
{
    return mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, NULL);
}

ssize_t mq_timedreceive(mqd_t mqdes, char *msg_ptr, size_t msg_len,
                        unsigned int *msg_prio,
                        const struct timespec *abs_timeout)
{
    size_t             msg_size_rcvd;
    int                priority;
    pthread_i          *ltcb;
//    sigevent           evp;
//    struct itimerspec  ivalue;
    mq                 *mq_node;
    struct sched_param param;
    int                policy;
    unsigned int       sig_recv = NULL;
//    timer_t            timerid;
    
    if (!IS_MQ_DESC_VALID(mqdes))
    {
        errno = EBADF;
        return -1;
    }

    mq_node = mqlist[mqdes];
    if (!mq_node)
    {
        errno = EINVAL;
        return -1;
    }
    
    if (mq_node->mq_attr.mq_flags & O_NONBLOCK) /* non-blocking mq if no msg */
    {
        /* if msg exists, we grab it and return */
        if (0 == mqlist_msg_get(mqdes, msg_ptr, msg_len, msg_prio, &msg_size_rcvd, 0, 0))
            return msg_size_rcvd;

        errno = EAGAIN;
        return -1;
    }

    /* if timeout is zero, return with TIMEOUT */
    if (abs_timeout && (0 == abs_timeout->tv_sec) && (0 == abs_timeout->tv_nsec))
    {
        /* if msg exists, we grab it and return */
        if (0 == mqlist_msg_get(mqdes, msg_ptr, msg_len, msg_prio, &msg_size_rcvd, 0, 0))
            return msg_size_rcvd;
        else
        {
            errno = ETIMEDOUT;
            return -1;
        }
    }

    if (0 != _getltcb_self(&ltcb))
    {
        errno = EINVAL;
        return -1;
    }

    pthread_getschedparam(ltcb->pthread, &policy, &param);

    priority = param.sched_priority;

    /* pass thread+priority info for use to setup blocking if no msg */
    if (0 == mqlist_msg_get(mqdes, msg_ptr, msg_len, msg_prio, &msg_size_rcvd,
                            ltcb->pthread, priority))
        return msg_size_rcvd;

    if (abs_timeout)
	{
        /* create and start timer */
//        evp.sigev_notify            = SIGEV_SIGNAL;
//        evp.sigev_signo             = SIGALRM;
//        evp.sigev_value.sival_int   = SIGALRM;
//        evp.sigev_notify_function   = 0;
//        evp.sigev_notify_attributes = 0;
//
//        if(0 != timer_create(0, &evp, &timerid))
//        {
//            errno = EFAULT;
//            return -1;
//        }
//
//        ivalue.it_value.tv_sec     = abs_timeout->tv_sec;
//        ivalue.it_value.tv_nsec    = abs_timeout->tv_nsec;
//        ivalue.it_interval.tv_sec  = 0;
//        ivalue.it_interval.tv_nsec = 0;
//
//        if(0 != timer_settime(timerid, 0, &ivalue, NULL))
//        {
//            errno = EFAULT;
//            return -1;
//        }
//
//        sig_recv = blast_anysignal_wait(&ltcb->sigs, POSIX_MQ_NOTIF_MASK | POSIX_SIGALARM_MASK);
    }
	else
	{
        sig_recv = blast_anysignal_wait(&ltcb->sigs, POSIX_MQ_NOTIF_MASK);
	}
	
    if (POSIX_SIGALARM_MASK == sig_recv)
    {
        /* time out */
	//  ECOMM or EINTR?
        errno = (abs_timeout) ? ETIMEDOUT : EINTR;
        return -1;
    }
    else if (POSIX_MQ_NOTIF_MASK == sig_recv)
    {
        blast_anysignal_clear(&ltcb->sigs, POSIX_MQ_NOTIF_MASK);
    
        /* now we try to go and get it */
        if (0 == mqlist_msg_get(mqdes, msg_ptr, msg_len, msg_prio, &msg_size_rcvd, 0, 0))
            return msg_size_rcvd;
        else
            return -1;
    }
    else
    {
        //something wrong with BLAST signal
        errno = EINTR;
        return -1;
    }
}

int select(int nfds, fd_set *restrict readfds,
           fd_set *restrict writefds, fd_set *restrict errorfds,
           struct timeval *restrict timeout)
{
    struct timespec timespec_timeout;

    if (NULL != timeout)
    {
        timespec_timeout.tv_sec  = timeout->tv_sec;
        timespec_timeout.tv_nsec = timeout->tv_usec * 1000;
        return pselect(nfds, readfds, writefds, errorfds, &timespec_timeout, NULL);
    }
    else
    {
        return pselect(nfds, readfds, writefds, errorfds, NULL, NULL);
    }
}

/* remove the calling thread from all waiting list of mqs in nfds */
static int _pselect_remove_waiting(unsigned char* mqs, pthread_t thread)
{
    int          fd;
    mq           *p_mq;
    unsigned int index = 0;

    while((fd=mqs[index++]) || index == MAX_MQ_PER_THREAD)
    {
        p_mq = mqlist[fd];
        if (p_mq)
        {
            (void) pthread_mutex_lock(p_mq->mq_lock);
            mqlist_remove_thread(p_mq, thread, 0, 0);
            (void) pthread_mutex_unlock(p_mq->mq_lock);
        }
    }
    return 0;
}

/* ignore writefds and errorfds */
int pselect(int nfds, fd_set *restrict readfds,
            fd_set *restrict writefds, fd_set *restrict errorfds,
            const struct timespec *restrict timeout,
            const sigset_t *restrict sigmask)
{
    int                fd;
    fd_set             returnfds;
    pthread_i          *ltcb;
    int                ret;
    int                priority;
    int                total_fds = 0;
    sigset_t           matched_sigs, sig;
    int                signo;
    unsigned int       i;
    struct sigaction   *sigaction_ptr;
    int                poll           = 0; /* don't poll, block by default */
    int                retry          = 1;
    struct sched_param param;
    int                policy;
    unsigned char      selected_mqs[MAX_MQ_PER_THREAD];   /* 8 words per thread */
    unsigned int       index = 0;
    pthread_t          thread;

    /* do not care write and error fds */

    if (0 == readfds)
        return -1;

    /* init return fds and wait fds */
    FD_ZERO(&returnfds);

    if (0 != _getltcb_self(&ltcb))
        return -1;

    thread = ltcb->pthread;

    if ((NULL != timeout) && (0 == timeout->tv_nsec) && (0 == timeout->tv_sec))
        poll = 1;

    if (NULL != sigmask)
    {
        /* check pending signals and compare with sigmask */
        matched_sigs = ltcb->sigpending & *sigmask;
        if (matched_sigs)
        {
            for (i = 0; i < sizeof(sigset_t) * 8; i++)
            {
                sig   = 1 << i;
                signo = i + 1;

                if (matched_sigs & sig)
                {
                    /* clear this pending signal */
                    ltcb->sigpending ^= sig;

                    /* call registered sig's signal handler */
                    sigaction_ptr = (struct sigaction *) ltcb->sigaction_ptrs[signo];

                    if ((signo < sizeof(sigset_t) * 8) && NULL != sigaction_ptr)
                    {
                        if (sigaction_ptr->sa_flags & SA_SIGINFO)
                        {
                            //call sa_sigaction
                            if (sigaction_ptr->sa_sigaction)
                                sigaction_ptr->sa_sigaction(signo, NULL, NULL);
                            else
                                return -1;
                        }
                        else
                        {
                            if (sigaction_ptr->sa_handler)
                                sigaction_ptr->sa_handler(signo);
                            else
                                return -1;
                        }
                    }
                    else
                        return -1;
                }
            }
            goto done_signal;
        }
    }

    thread = ltcb->pthread;
    pthread_getschedparam(ltcb->pthread, &policy, &param);
    priority = param.sched_priority;

    /* malloc the select_mask for this thread. Only free it when the thread exit */
    if (0 == ltcb->select_mask)
    {
        ltcb->select_mask = (fd_set*)malloc(sizeof(fd_set));
        FD_ZERO(ltcb->select_mask);
    }

    /* prepare the array which stores selected mqs */
    for (i=0; i< howmany(FD_SETSIZE, NFDBITS); i++)
    {
        unsigned char bit;

        while ((bit = Q6_R_ct0_R(readfds->fds_bits[i])) < 32 && index < MAX_MQ_PER_THREAD)
        {
            fd = selected_mqs[index++] = bit+(i<<5); /* i<<5 == i*32 */;
            if (mqlist_msg_exists(fd, thread, priority))
            {
                /* clear the bit in select_mask to avoid later mismatch between select_mask and mq status */
                FD_CLR(fd, ltcb->select_mask); 
                FD_SET(fd, &returnfds);
                total_fds++;
            }            

            /* clr the bit in the word */
            readfds->fds_bits[i] = Q6_R_clrbit_RR(readfds->fds_bits[i], bit);
        }
        if (index == MAX_MQ_PER_THREAD)
        {
            break;
        }
    }
    if (index < MAX_MQ_PER_THREAD)
    {
        selected_mqs[index] = 0; /* 0 inidicate the end of mq list */
    }

    if (total_fds > 0 || poll)
    {
        /* we either got one or more msgs, or the caller does nto want to block */
        goto done_mq;
    }
    else
    {
        while (retry)
        {
            retry = 0;

            FD_ZERO(readfds);

            /* set sigmask */
            ltcb->sigwaiting = (sigmask == NULL ? 0 : *sigmask);
            
            /* we did not get any msg. Will block here */
            ret = blast_anysignal_wait(&ltcb->sigs, POSIX_MQ_NOTIF_MASK | POSIX_SIGNAL_MASK);

            if (POSIX_SIGNAL_MASK & ret)
            {
                blast_anysignal_clear(&ltcb->sigs, POSIX_SIGNAL_MASK);
                
                sig              = ltcb->sigpending;
                signo            = ffs(sig);
                ltcb->sigpending = 0;

                /* clear this pending signal */
                ltcb->sigpending ^= sig;

                /* call signal handler for this signal */
                sigaction_ptr = (struct sigaction *) ltcb->sigaction_ptrs[signo];

                /* if user has registered through sigaction, then invoke the handler */
                if ((signo < sizeof(sigset_t) * 8) && NULL != sigaction_ptr)
                {
                    if (sigaction_ptr->sa_flags & SA_SIGINFO)
                    {
                        //call sa_sigaction
                        if (sigaction_ptr->sa_sigaction)
                            sigaction_ptr->sa_sigaction(signo, NULL, NULL);
                        else
                            return -1;
                    }
                    else
                    {
                        if (sigaction_ptr->sa_handler)
                            sigaction_ptr->sa_handler(signo);
                        else
                            return -1;
                    }
                }
                else
                    return -1;

                goto done_signal;
            }

            if (POSIX_MQ_NOTIF_MASK & ret )
            {
                blast_anysignal_clear(&ltcb->sigs, POSIX_MQ_NOTIF_MASK);
            
                /* search for the all queues for message */
                index = 0;
                while((fd=selected_mqs[index++]) || index == MAX_MQ_PER_THREAD)
                {
                    if (FD_ISSET(fd, ltcb->select_mask))
                    {
                        if (mqlist_msg_exists(fd, 0, 0))
                        {
                            /* now we really got a msg */
                            total_fds++;
                            FD_CLR(fd, ltcb->select_mask);
                            FD_SET(fd, readfds);                            
                        }
                        else                        
                        {
                            /* we got an false signal and select_mask, ignore it */
                            FD_CLR(fd, ltcb->select_mask);                            
                        }
                    }
                }

                if( total_fds == 0 )
                {
                    mq_fd_err_cnt++;
                    retry = 1;
                }
                else
                {
                    /* another done_mq */
  
                    /* we got msg and will return. Remove this thread from all p_mq */
                    (void) _pselect_remove_waiting(selected_mqs, thread);
                    return total_fds;
                }
            }
            if( !retry )
            {
                //something wrong with BLAST signal
                errno = EINVAL;
  
                /* we got msg and will return. Remove this thread from all p_mq */
                (void) _pselect_remove_waiting(selected_mqs, thread);
  
                return -1;
            }
        } //while
    }     //if (total_fds > 0 || 0 == blocking )

 done_mq:
    /* we got msg and will return. Remove this thread from all p_mq */
    (void) _pselect_remove_waiting(selected_mqs, thread);
    FD_COPY(&returnfds, readfds);
    return total_fds;

 done_signal:
    /* we got msg and will return. Remove this thread from all p_mq */
    (void) _pselect_remove_waiting(selected_mqs, thread);

    errno = EINTR;
    return -1;

// badf:
  //  return 0;
}
int mq_getattr(mqd_t mqdes, struct mq_attr *mqstat)
{
    mq * mq_node = mqlist[mqdes];

    if (!mqstat || !mq_node)
    {
        errno = EBADF;
        return -1;
    }

    (void) pthread_mutex_lock(mq_node->mq_lock);

    memcpy(mqstat, &(mq_node->mq_attr), sizeof(struct mq_attr));

    (void) pthread_mutex_unlock(mq_node->mq_lock);

    return 0;
}

int mq_setattr(mqd_t mqdes, const struct mq_attr *mqstat,
               struct mq_attr *omqstat)
{
    int ret       = -1;
    mq  * mq_node = mqlist[mqdes];

    if (!mqstat || !mq_node)
    {
        errno = EBADF;
        return -1;
    }

    (void) pthread_mutex_lock(mq_node->mq_lock);

    if (omqstat)
    {
        memcpy(omqstat, &(mq_node->mq_attr), sizeof(struct mq_attr));
    }

    if (mqstat->mq_flags != O_NONBLOCK)
    {
        errno = EINVAL;
    }
    else
    {
        mq_node->mq_attr.mq_flags = mqstat->mq_flags;
        ret                       = 0;
    }

    (void) pthread_mutex_unlock(mq_node->mq_lock);

    return ret;
}

