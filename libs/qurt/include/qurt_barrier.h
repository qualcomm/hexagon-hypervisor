/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_BARRIER_H
#define QURT_BARRIER_H

/**
  @file qurt_barrier.h
  @brief  Prototypes of Kernel barrier  API functions      

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

/** @addtogroup barrier_types
@{ */

#define QURT_BARRIER_SERIAL_THREAD 1
#define QURT_BARRIER_OTHER 0

#include <h2.h>
#ifndef ASM
#include <qurt_mutex.h>

/** qurt_barrier type.                                                 
 
  All threads are woken up only when all threads have reached the
  qurt_barrier_wait call.
 */
typedef h2_barrier_t qurt_barrier_t;

/** @} */ /* end_addtogroup barrier_types */

 

static inline int qurt_barrier_init(qurt_barrier_t *barrier, unsigned int threads_total)
{
	return h2_barrier_init(barrier,threads_total);
}

static inline int qurt_barrier_destroy(qurt_barrier_t *barrier) { return QURT_EOK; }

static inline int qurt_barrier_wait(qurt_barrier_t *barrier)
{
	return h2_barrier_wait(barrier);
}

#endif

#endif /* QURT_BARRIER_H */

