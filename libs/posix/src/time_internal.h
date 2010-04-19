/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef _TIME_INTERNAL_H_
#define _TIME_INTERNAL_H_

#include <sys/types.h>
#include <blast.h>
//#include <qube.h>
//#include <qtimer.h> //for Qtimer on BLAST

//#define SIM_TIMER_HACK 1

typedef struct timer_i   timer_i;
struct timer_i
{

#if (0)
    qtimer_t          qtimer;
    qtimer_attr_t     qtimer_attr;
    /* flag to decide to change reload time for periodic timer */
    int               need_reload;
    qtimer_duration_t reload_time;
    pthread_t         thread;
    struct sigevent   *evp;
#endif

};

#endif //_TIME_INTERNAL_H_
