/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <qurt.h>
#include <qurt_timer.h>
#include <signal.h>
#include <common/time.h>
#include <sys/errno.h>
#include "time_internal.h"
#include "pthread_internal.h"

#ifdef __QDSP6_ARCH__
#include <q6protos.h>
#else
#include <hexagon_protos.h>
#endif
#define ffs(x) (Q6_R_ct0_R(x)+1)

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
        (void) qurt_timer_delete(timer->qurt_timer);
        qurt_timer_attr_set_duration(&timer->qurt_timer_attr, timer->reload_time);
        timer->reload_time = 0;

        if (EOK != qmsgq_callback_init(&timer->qurt_timer_cb, ltcb->qmsgq_notif, SIGALRM, (void*) addr, MSG_SIZE))
            return -1;

        if (EOK != qurt_timer_create(&timer->qurt_timer, &timer->qurt_timer_attr, &timer->qurt_timer_cb))
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

int return_sigwait(sigset_t *pending, const sigset_t *restrict set, int *restrict sig)
{
    int           lowest_sig_no = 0;

    //reach this point if we have a pending signal, else error
    lowest_sig_no = ffs(*pending & *set);

    //clear this pending signal
    (void) sigdelset(pending, lowest_sig_no);

    //only report back the lowest occured signal
    *sig = lowest_sig_no;
    return 0;    
}

int sigwait(const sigset_t *restrict set, int *restrict sig)
{
    pthread_i     *ltcb;
    unsigned int  recv_mask, qurt_sigmask;
#ifdef CONFIG_QTIMERS
    unsigned int  qurt_signo;
    int           posix_signo;
    timer_i       * timer;
    qurt_timer_type_t type;
#endif
    int           wait_again = 1;
    volatile sigset_t *sigpending;
    int           ret = _getltcb_self(&ltcb);

    if (ret != 0 || !ltcb || !set || !sig)
    {
        errno = EINVAL;
        return -1;
    }

    sigpending = &ltcb->sigpending;

    if ((*sigpending & *set) == 0)
    {
        ltcb->sigwaiting = *set;

        /* check sigpending second time to avoid race condition */
        if ((*sigpending & *set) != 0)
        {
            return return_sigwait(&ltcb->sigpending, set, sig);
        }

        while (wait_again)
        {
            wait_again = 0;
        recv_mask     = qurt_anysignal_wait(&ltcb->sigs, POSIX_SIGNAL_MASK | POSIX_TIMER_MASK);
        qurt_sigmask = ffs_mask(recv_mask);

        /* receive a normal signal */
        if (POSIX_SIGNAL_MASK == qurt_sigmask)
        {
            qurt_anysignal_clear(&ltcb->sigs, POSIX_SIGNAL_MASK);
                //received a empty signal, wait again
                if ((*sigpending & *set) == 0)
                { 
                    wait_again = 1;
                }
        }

        /* receive a timer expiration signal */
        else if (qurt_sigmask >= POSIX_TIMER_SIGNO_MIN_MASK || qurt_sigmask <= POSIX_TIMER_SIGNO_MAX_MASK)
        {
        #ifdef CONFIG_QTIMERS
            do /* to get all timer signals at qurt level */
            {
                qurt_anysignal_clear(&ltcb->sigs, qurt_sigmask);
                qurt_signo = ffs(recv_mask);
                if (qurt_signo < POSIX_TIMER_SIGNO_MIN || qurt_signo > POSIX_TIMER_SIGNO_MAX)
                {
                    errno = EINVAL;
                    return -1;
                }

                /* get timer pointer */
                timer = ltcb->timers[qurt_signo - POSIX_TIMER_SIGNO_MIN];

                /* map to POSIX signo */
                if (NULL == timer)
                {
                    errno = EINVAL;
                    return -1;
                }

                /* get the signo at posix level */
                posix_signo = timer->evp->sigev_signo;

                /* add the pending timer signal at pthread mask */
                (void) sigaddset(&ltcb->sigpending, posix_signo);

                qurt_timer_attr_get_type(&timer->qurt_timer_attr, &type);
                if (QURT_TIMER_ONESHOT == type)
                {
                    /* clear the timer entry in ltcb timer list */
                    ltcb->timers[qurt_signo - POSIX_TIMER_SIGNO_MIN] = NULL;
                }
                else if (QURT_TIMER_PERIODIC == type)
                {
                    /* create the timer with reload time first time */
                    if (timer->need_reload)
                    {
                        if (EOK != qurt_timer_delete(timer->qurt_timer))
                        {
                            errno = EINVAL;
                            return -1;
                        }

                        qurt_timer_attr_set_duration(&timer->qurt_timer_attr, timer->reload_time);
                        timer->reload_time = 0;
                        timer->need_reload = 0; /* only reload once */

                        if (EOK != qurt_timer_create(&timer->qurt_timer, &timer->qurt_timer_attr, &ltcb->sigs,
                                                     qurt_sigmask))
                        {
                            errno = EINVAL;
                            return -1;
                        }
                    }
                }
                else /* invalid timer type */
                {
                    errno = EINVAL;
                    return -1;
                }

                /* prepare to check next bit */
                recv_mask     = recv_mask & (~qurt_sigmask);
                qurt_sigmask = ffs_mask(recv_mask);
            } while (recv_mask);
        #else
            errno = EINVAL;
            return -1;
        #endif
        }
        /* recevied an unwanted QURT signal */
        else
        {
            errno = ENOSYS;
            return -1;
        }
    }
    }

    return return_sigwait(&ltcb->sigpending, set, sig);
}

int sigsuspend(const sigset_t *sigmask)
{
    struct sigaction * sigaction_ptr = 0;
    pthread_i        *ltcb;
    int              sig = 0;

    if (!sigmask || (0 != _getltcb_self(&ltcb)) || NULL == ltcb)
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

