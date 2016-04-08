/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_ALLSIGNAL_H
#define QURT_ALLSIGNAL_H

/**
  @file  qurt_allsignal.h
  @brief  Prototypes of Kernel signal API functions      

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

#include <qurt_signal.h>

/** @addtogroup all_signal_types
@{ */

/** qurt_allsignal type.         
 The caller of signal_wait can only be woken up when all bits in the mask are set. */
typedef qurt_signal_t qurt_allsignal_t;
/** @} */ /* end_addtogroup chapter_all_signal */

 

static inline void qurt_allsignal_init(qurt_allsignal_t *signal)
{
	return qurt_signal_init(signal);
}

static inline void qurt_allsignal_destroy(qurt_allsignal_t *signal) {}

static inline unsigned int qurt_allsignal_get(qurt_allsignal_t *signal)
{ return qurt_signal_get(signal); };
    

static inline void qurt_allsignal_wait(qurt_allsignal_t *signal, unsigned int mask)
{
	qurt_signal_wait_all(signal,mask);
	qurt_signal_clear(signal,mask);
}

static inline void qurt_allsignal_set(qurt_allsignal_t *signal, unsigned int mask)
{
	return qurt_signal_set(signal,mask);
}

static inline void qurt_allsignal_clear(qurt_allsignal_t *signal, unsigned int mask) { qurt_signal_clear(signal,mask); }

#endif /* QURT_ALLSIGNAL_H */

