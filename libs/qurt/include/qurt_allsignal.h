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

#include <h2.h>
#include <qurt_prelim.h>

/** @addtogroup all_signal_types
@{ */

/** qurt_allsignal type.         
 The caller of signal_wait can only be woken up when all bits in the mask are set. */
typedef h2_allsignal_t qurt_allsignal_t;
/** @} */ /* end_addtogroup chapter_all_signal */

 

static inline void qurt_allsignal_init(qurt_allsignal_t *signal)
{
	return h2_allsignal_init(signal);
}

static inline void qurt_allsignal_destroy(qurt_allsignal_t *signal) {}

static inline unsigned int qurt_allsignal_get(qurt_allsignal_t *signal)
{ return signal->signals_in; };
    

static inline void qurt_allsignal_wait(qurt_allsignal_t *signal, unsigned int mask)
{
	return h2_allsignal_wait(signal,mask);
}

static inline void qurt_allsignal_set(qurt_allsignal_t *signal, unsigned int mask)
{
	return h2_allsignal_signal(signal,mask);
}

static inline void qurt_allsignal_clear(qurt_allsignal_t *signal, unsigned int mask) { UNSUPPORTED; }

#endif /* QURT_ALLSIGNAL_H */

