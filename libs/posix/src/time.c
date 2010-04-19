/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <common/time.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include "time_internal.h"
#include "pthread_internal.h"

static sem_t timer_worker_sem;
static int   timer_worker_sem_inited = 0;

/* Internal thread processing the msgq */
static void * _timer_worker(void *parg)
{

#if (0)
    int           status = 0;
    pthread_i     * ltcb;
    qtimer_type_t type;
    unsigned int  timer_sigmask = 0;
    int           i;
    unsigned int  recv_mask;
    unsigned int  blast_signo;

    /* wait for sem before move forward */
    if (0 != sem_wait(&timer_worker_sem))
        return 0;

    timer_i *timer = (timer_i *) parg;

    if (!timer)
        return 0;

    if (0 != _getltcb_self(&ltcb) || !ltcb)
        return 0;

    /* find an empty spot in tiemr list of this thread */
    for (i = 0; i < POSIX_MAX_TIMER_NUM; i++)
    {
        if (ltcb->timers[i] == NULL)
        {
            ltcb->timers[i] = timer;
            timer_sigmask   = 1 << (POSIX_SIGALARM_KERNEL_SIGNO + i - 1);
            break;
        }
    }

    /* reach the limit of the number of timers per thread */
    if (i == POSIX_MAX_TIMER_NUM)
        return 0;

    if (EOK != sclk_timer_create(&timer->qtimer, &timer->qtimer_attr, &ltcb->sigs,
                                 timer_sigmask))
        return 0;

    for (;;)
    {
        recv_mask = blast_anysignal_wait(&ltcb->sigs, POSIX_TIMER_MASK);
        if (recv_mask >= POSIX_TIMER_SIGNO_MIN_MASK || recv_mask <= POSIX_TIMER_SIGNO_MAX_MASK)
        {
            blast_anysignal_clear(&ltcb->sigs, recv_mask);
            blast_signo = ffs(recv_mask);
            if (blast_signo < POSIX_TIMER_SIGNO_MIN || blast_signo > POSIX_TIMER_SIGNO_MAX)
            {
                return 0;
            }

            timer = ltcb->timers[blast_signo - POSIX_TIMER_SIGNO_MIN];

            if (timer && timer->evp->sigev_notify_function)
                /* invoke the callback function */
                timer->evp->sigev_notify_function(timer->evp->sigev_value);
            else
                return 0;

            /* depending on the type of timer, either return or continue to receive */
            qtimer_attr_gettype(&timer->qtimer_attr, &type);
            if (QTIMER_ONESHOT == type)
            {
                ltcb->timers[blast_signo - POSIX_TIMER_SIGNO_MIN] = NULL;
                pthread_exit((void *) &status);
                return 0;
            }
            else
            {
                /* create the timer with reload time first time */
                if (timer->need_reload)
                {
                    if (EOK != qtimer_delete(timer->qtimer))
                        return 0;

                    qtimer_attr_setduration(&timer->qtimer_attr, timer->reload_time);
                    timer->reload_time = 0;
                    timer->need_reload = 0; /* only reload once */

                    if (EOK != sclk_timer_create(&timer->qtimer, &timer->qtimer_attr, &ltcb->sigs,
                                                 recv_mask))
                        return 0;
                }
            }
        } //end of if
    }     //end of for loop
#endif
}

int timer_create(clockid_t clockid, struct sigevent *restrict evp,
                 timer_t *restrict timerid)
{
    timer_i * timer;

    //unused param
    clockid = clockid;

    if (NULL == timerid)
        return -1;

    timer = (timer_i *) malloc(sizeof(timer_i));
    if (NULL == timer)
        return -1;

    memset(timer, 0, sizeof(timer_i));

    timer->evp = (sigevent *) malloc(sizeof(sigevent));
    if (!timer->evp)
    {
        free(timer);
        return -1;
    }

    if (NULL == evp)
    {
        timer->evp->sigev_notify            = SIGEV_SIGNAL;
        timer->evp->sigev_signo             = SIGALRM;     /* defalut Signal */
        timer->evp->sigev_value.sival_int   = (int) timer; /* Signal value. default is timer_id */
        timer->evp->sigev_notify_function   = 0;
        timer->evp->sigev_notify_attributes = 0;
    }
    else
    {
        memcpy(timer->evp, evp, sizeof(sigevent));
    }

    *timerid = timer;
    return 0;
}

#define EOK 0

int timer_delete(timer_t timerid)
{
    timer_i * timer = (timer_i *) timerid;

    if (!timer)
        return -1;

    if (timer->qtimer)
    {
        if (EOK != qtimer_delete(timer->qtimer))
            return -1;
    }

    if (timer->thread)
    {
        if (0 != pthread_cancel(timer->thread))
            return -1;
    }

    if (timer->evp)
        free(timer->evp);

    free(timer);
    return 0;
}

int timer_gettime(timer_t timerid, struct itimerspec *value)
{
#if (0)
    timer_i           * timer = (timer_i *) timerid;
    qtimer_duration_t time;

    if (!timer)
        return -1;

    if (timer->qtimer)
    {
        qtimer_attr_t attr;

        if (EOK != qtimer_get_attr(timer->qtimer, &attr))
            return -1;

        qtimer_attr_getremaining(&attr, &time);

        value->it_interval.tv_sec  = time / 1000000;
        value->it_interval.tv_nsec = (time % 1000000) * 1000;

        return 0;
    }
    return -1;
#endif
}

int timer_settime(timer_t timerid, int flags, const struct itimerspec *restrict value,
                  struct itimerspec *restrict ovalue)
{
#if (0)
    timer_i           * timer = (timer_i *) timerid;
    qtimer_duration_t time_duration;
    qtimer_type_t     type          = QTIMER_ONESHOT;
    unsigned int      timer_sigmask = 0;
    int               i;

    //unused param
    flags  = flags;
    ovalue = ovalue;

    if (!timer || !value)
        return -1;

    /*lint -e647 Suspicious truncation*/
    /* timer expiry duration from the time timer is created */
    time_duration =
        (value->it_value.tv_sec) * 1000000 + (value->it_value.tv_nsec) / 1000;

    /* timer is periodic if interval is specified */
    if (value->it_interval.tv_sec || value->it_interval.tv_nsec)
    {
        timer->reload_time =
            (value->it_interval.tv_sec) * 1000000 + (value->it_interval.tv_nsec) / 1000;
        type = QTIMER_PERIODIC;

        /* need to reload the time value when the timer expires first time */
        if (time_duration != timer->reload_time)
        {
            timer->need_reload = 1;
        }
    }
    /*lint +e647 Suspicious truncation*/

    /* setup timer attributes */
    qtimer_attr_init(&timer->qtimer_attr);
    qtimer_attr_settype(&timer->qtimer_attr, type);
    qtimer_attr_setduration(&timer->qtimer_attr, time_duration);

    /* is the timer handled by a separate thread ? */
    if (SIGEV_THREAD == timer->evp->sigev_notify)
    {
        if (0 == timer_worker_sem_inited)
        {
            if (0 != sem_init(&timer_worker_sem, 0, 0))
                return -1;
            else
                timer_worker_sem_inited = 1;
        }

        /* create the thread per timer creation */
        if (0 != pthread_create(&timer->thread, timer->evp->sigev_notify_attributes, _timer_worker, (void *) timer))
            return -1;

        /* let work thread move forward */
        if (0 != sem_post(&timer_worker_sem))
            return -1;
    }
    /* the client thread is going to sigwait() after timer_settime call() */
    else if (SIGEV_SIGNAL == timer->evp->sigev_notify)
    {
        pthread_i * ltcb;

        if (0 != _getltcb_self(&ltcb) || !ltcb)
            return -1;

        /* find an empty spot in tiemr list of this thread */
        for (i = 0; i < POSIX_MAX_TIMER_NUM; i++)
        {
            if (ltcb->timers[i] == NULL)
            {
                ltcb->timers[i] = timer;
                timer_sigmask   = 1 << (POSIX_SIGALARM_KERNEL_SIGNO + i - 1);
                break;
            }
        }

        /* reach the limit of the number of timers per thread */
        if (i == POSIX_MAX_TIMER_NUM)
            return -1;

        if (EOK != sclk_timer_create(&timer->qtimer, &timer->qtimer_attr, &ltcb->sigs,
                                     timer_sigmask))
            return -1;
    }
    else
    {
        return -1;
    }

    /* not sure of what is to be done with ovalue */
    /* *ovalue = 0; */
#endif
    return 0;
}
