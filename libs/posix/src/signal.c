/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast.h>
//#include <qube.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include "time_internal.h"
#include "pthread_internal.h"

int ffs_mask(register unsigned int value)
{
    register int i = 0;
    if (value)
        for (i = 1; ~value & 1; value = value >> 1, ++i) ;
    return 1 << (i - 1);
}

int _handle_sigalarm(pthread_i *ltcb, unsigned int *msg)
{
#if 0
    timer_i *timer = 0;

    if (!msg || !ltcb)
        return -1;

    timer = (timer_i*) *msg;
    if (timer && timer->reload_time) /* periodic timer, create the timer with reload time */
    {
        unsigned int * addr = 0;
        addr = &timer;

        /* set the reload time only once */
        (void) qtimer_delete(timer->qtimer);
        qtimer_attr_setduration(&timer->qtimer_attr, timer->reload_time);
        timer->reload_time = 0;

        if (EOK != qmsgq_callback_init(&timer->qtimer_cb, ltcb->qmsgq_notif, SIGALRM, (void*) addr, MSG_SIZE))
            return -1;

        if (EOK != qtimer_create(&timer->qtimer, &timer->qtimer_attr, &timer->qtimer_cb))
            return -1;
    }

    if (EOK != qmsgq_free_message((void*) msg))
        return -1;
#endif

    return 0;
}

int _sigaction(int sig, const struct sigaction *act, struct sigaction *oact)
{
    pthread_i        *ltcb           = 0;
    struct sigaction * sigaction_ptr = 0;
    int              ret             = _getltcb_self(&ltcb);

    if (ret != 0 || !ltcb || !act || sig > 31)
        return -1;

    sigaction_ptr = (struct sigaction *) ltcb->sigaction_ptrs[sig];
    if (oact && sigaction_ptr)
        *oact = *sigaction_ptr;
    if (act)
        ltcb->sigaction_ptrs[sig] = (void *) act;
    return 0;
}

//  See if we get away with not fully supporting this.

int sigwait(const sigset_t *restrict set, int *restrict sig)
{
    pthread_i     *ltcb;
    int           lowest_sig_no = 0;
    unsigned int  recv_mask, blast_sigmask;
//    unsigned int  blast_signo;
//    int           posix_signo;
//    timer_i       * timer;
//    qtimer_type_t type;

    int           ret = _getltcb_self(&ltcb);
    if (ret != 0 || !ltcb || !set || !sig)
    {
        errno = EINVAL;
        return -1;
    }

    if ((ltcb->sigpending & *set) == 0)
    {
        ltcb->sigwaiting = *set;

        recv_mask     = blast_anysignal_wait(&ltcb->sigs, POSIX_SIGNAL_MASK | POSIX_TIMER_MASK);
        blast_sigmask = ffs_mask(recv_mask);

        /* receive a normal signal */
        if (POSIX_SIGNAL_MASK == blast_sigmask)
        {
            blast_anysignal_clear(&ltcb->sigs, POSIX_SIGNAL_MASK);
        }
        /* receive a timer expiration signal */
        else if (blast_sigmask >= POSIX_TIMER_SIGNO_MIN_MASK || blast_sigmask <= POSIX_TIMER_SIGNO_MAX_MASK)
        {
//            do /* to get all timer signals at blast level */
//            {
//                blast_anysignal_clear(&ltcb->sigs, blast_sigmask);
//                blast_signo = ffs(recv_mask);
//                if (blast_signo < POSIX_TIMER_SIGNO_MIN || blast_signo > POSIX_TIMER_SIGNO_MAX)
//                {
//                    errno = EINVAL;
//                    return -1;
//                }
//
//                /* get timer pointer */
//                timer = ltcb->timers[blast_signo - POSIX_TIMER_SIGNO_MIN];
//
//                /* map to POSIX signo */
//                if (NULL == timer)
//                {
//                    errno = EINVAL;
//                    return -1;
//                }
//
//                /* get the signo at posix level */
//                posix_signo = timer->evp->sigev_signo;
//
//                /* add the pending timer signal at pthread mask */
//                (void) sigaddset(&ltcb->sigpending, posix_signo);
//
//                qtimer_attr_gettype(&timer->qtimer_attr, &type);
//                if (QTIMER_ONESHOT == type)
//                {
//                    /* clear the timer entry in ltcb timer list */
//                    ltcb->timers[blast_signo - POSIX_TIMER_SIGNO_MIN] = NULL;
//                }
//                else if (QTIMER_PERIODIC == type)
//                {
//                    /* create the timer with reload time first time */
//                    if (timer->need_reload)
//                    {
//                        if (EOK != qtimer_delete(timer->qtimer))
//                        {
//                            errno = EINVAL;
//                            return -1;
//                        }
//
//                        qtimer_attr_setduration(&timer->qtimer_attr, timer->reload_time);
//                        timer->reload_time = 0;
//                        timer->need_reload = 0; /* only reload once */
//
//                        if (EOK != sclk_timer_create(&timer->qtimer, &timer->qtimer_attr, &ltcb->sigs,
//                                                     blast_sigmask))
//                        {
//                            errno = EINVAL;
//                            return -1;
//                        }
//                    }
//                }
//                else /* invalid timer type */
//                {
//                    errno = EINVAL;
//                    return -1;
//                }
//
//                /* prepare to check next bit */
//                recv_mask     = recv_mask & (~blast_sigmask);
//                blast_sigmask = ffs_mask(recv_mask);
//            } while (recv_mask);
        }
        /* recevied an unwanted BLAST signal */
        else
        {
            errno = ENOSYS;
            return -1;
        }
    }

    //reach this point if we have a pending signal, else error
    lowest_sig_no = ffs(ltcb->sigpending & *set);

    //clear this pending signal
    (void) sigdelset(&ltcb->sigpending, lowest_sig_no);

    //only report back the lowest occured signal
    *sig = lowest_sig_no;
    return 0;
}

int sigsuspend(const sigset_t *sigmask)
{
    struct sigaction * sigaction_ptr = 0;
    pthread_i        *ltcb;
    int              sig = 0;

    if (!sigmask || (0 != _getltcb_self(&ltcb)))
        return -1;

    if (0 != sigwait(sigmask, &sig))
        return -1;

    sigaction_ptr = (struct sigaction *) ltcb->sigaction_ptrs[sig];

    //if user has registered through sigaction, then invoke the handler
    if ((sig < 32) && NULL != sigaction_ptr)
    {
        if (sigaction_ptr->sa_flags & SA_SIGINFO)
        {
            //call sa_sigaction
            if (sigaction_ptr->sa_sigaction)
                sigaction_ptr->sa_sigaction(sig, NULL, NULL);
            else
                return -1;
        }
        else
        {
            if (sigaction_ptr->sa_handler)
                sigaction_ptr->sa_handler(sig);
            else
                return -1;
        }
    }
    else
        return -1;
    return 0;
}

int sigaddset(sigset_t *set, int signo)
{
    if (NULL == set || signo > SIGRTMAX)
    {
        errno = EINVAL;
        return -1;
    }
    *(set) |= 1 << ((signo) - 1);
    return 0;
}

int sigdelset(sigset_t *set, int signo)
{
    if (NULL == set || signo > SIGRTMAX)
    {
        errno = EINVAL;
        return -1;
    }
    *set &= ~(1 << (signo - 1));
    return 0;
}

int sigemptyset(sigset_t *set)
{
    if (NULL == set)
    {
        errno = EINVAL;
        return -1;
    }
    *set = 0;
    return 0;
}

int sigfillset(sigset_t *set)
{
    if (NULL == set)
    {
        errno = EINVAL;
        return -1;
    }
    *set = ~(sigset_t) 0;
    return 0;
}

int sigismember(const sigset_t *set, int signo)
{
    if (NULL == set || signo > SIGRTMAX)
    {
        errno = EINVAL;
        return -1;
    }
    return((*set & (1 << (signo - 1))) != 0);
}

