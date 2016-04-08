/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_SIGNAL_H
#define QURT_SIGNAL_H

/**
  @file qurt_signal.h
  @brief  Prototypes of kernel signal API functions.

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/
#include <h2.h>

#define QURT_SIGNAL_ATTR_WAIT_ANY H2_SIGNAL_WAIT_ANY
#define QURT_SIGNAL_ATTR_WAIT_ALL H2_SIGNAL_WAIT_ALL

/** @addtogroup signals_types
@{ */
/** qurt_signal type                                           
 */
typedef h2_signal_t qurt_signal_t;
/** @} */ /* end_addtogroup signals_types */

 

static inline void qurt_signal_init(qurt_signal_t *signal) { h2_signal_init(signal); }

static inline void qurt_signal_destroy(qurt_signal_t *signal) {}

static inline unsigned int qurt_signal_wait_any(qurt_signal_t *signal, unsigned int mask)
{
	return h2_signal_wait_any(signal,mask);
}

static inline unsigned int qurt_signal_wait_all(qurt_signal_t *signal, unsigned int mask)
{
	return h2_signal_wait_all(signal,mask);
}

static inline unsigned int qurt_signal_wait(qurt_signal_t *signal, unsigned int mask, 
                unsigned int attribute)
{
	if (attribute == QURT_SIGNAL_ATTR_WAIT_ANY) {
		return qurt_signal_wait_any(signal,mask);
	} else {
		return qurt_signal_wait_all(signal,mask);
	}
}

static inline void qurt_signal_set(qurt_signal_t *signal, unsigned int mask)
{
	h2_signal_set(signal,mask);
}

static inline unsigned int qurt_signal_get(qurt_signal_t *signal)
{
	return h2_signal_get(signal);
}

static inline void qurt_signal_clear(qurt_signal_t *signal, unsigned int mask)
{
	h2_signal_clear(signal,mask);
}

/**@ingroup func_qurt_signal_wait_cancellable    
  Suspends the current thread until either the specified signals are set or the wait operation is cancelled.
  The operation is cancelled if the user process of the calling thread is killed, or if the calling thread must finish its current QDI invocation and return to user space. 

  Signals are represented as bits 0-31 in the 32-bit mask value. A mask bit value of 1 indicates that a signal is to be waited on, and 0 that it is not to be waited on.

  If a thread is waiting on a signal object for any of the specified set of signals to be set, and one or more of those signals is set in the signal object, then the thread is awakened.

  If a thread is waiting on a signal object for all of the specified set of signals to be set, and all of those signals are set in the signal object, then the thread is awakened.

  @note1hang At most one thread can wait on a signal object at any given time.

  @note1hang  When the operation is cancelled, the caller should assume that the signal is never going to be set.

  @datatypes
  #qurt_signal_t

  @param[in] signal      Pointer to the signal object to wait on.
  @param[in] mask        Mask value which identifies the individual signals in the signal object to be 
                         waited on.
  @param[in] attribute   Indicates whether the thread waits for any of the signals to be set, or for all of 
                         them to be set. Values:\n
                         - QURT_SIGNAL_ATTR_WAIT_ANY \n
                         - QURT_SIGNAL_ATTR_WAIT_ALL @tablebulletend
  @param[out] return_mask Pointer to the 32-bit mask value that was originally passed to the function.

  @return     	
  QURT_EOK -- Wait completed. \n
  QURT_ECANCEL -- Wait cancelled.

 
  @dependencies
  None.
*/
/* ======================================================================*/
static inline int qurt_signal_wait_cancellable(qurt_signal_t *signal, unsigned int mask, 
                                 unsigned int attribute,
                                 unsigned int *return_mask __attribute__((unused)))
{
	return qurt_signal_wait(signal,mask,attribute);
}

#endif /* QURT_SIGNAL_H */
