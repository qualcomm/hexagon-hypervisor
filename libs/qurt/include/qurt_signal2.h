/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_SIGNAL2_H
#define QURT_SIGNAL2_H

/**
  @file qurt_signal2.h
  @brief  Prototypes of kernel signal2 API functions.

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

#include <qurt_signal.h>

#define QURT_SIGNAL_ATTR_WAIT_ANY 0x00000000
#define QURT_SIGNAL_ATTR_WAIT_ALL 0x00000001

/** @addtogroup signals_types
@{ */
/** qurt_signal2 type                                           
 */
typedef qurt_signal_t qurt_signal2_t;
/** @} */ /* end_addtogroup signals_types */

 

static inline void qurt_signal2_init(qurt_signal2_t *signal) { qurt_signal_init(signal); }

static inline void qurt_signal2_destroy(qurt_signal2_t *signal) { qurt_signal_destroy(signal); }

static inline unsigned int qurt_signal2_wait(qurt_signal2_t *signal, unsigned int mask, 
                unsigned int attribute) { return qurt_signal_wait(signal,mask,attribute); }

static inline unsigned int qurt_signal2_wait_any(qurt_signal2_t *signal, unsigned int mask)
{
  return qurt_signal2_wait(signal, mask, QURT_SIGNAL_ATTR_WAIT_ANY);
}

static inline unsigned int qurt_signal2_wait_all(qurt_signal2_t *signal, unsigned int mask)
{
  return qurt_signal2_wait(signal, mask, QURT_SIGNAL_ATTR_WAIT_ALL);
}

static inline void qurt_signal2_set(qurt_signal2_t *signal, unsigned int mask) { return qurt_signal_set(signal,mask); }

static inline unsigned int qurt_signal2_get(qurt_signal2_t *signal) { return qurt_signal_get(signal); }

static inline void qurt_signal2_clear(qurt_signal2_t *signal, unsigned int mask) { return qurt_signal_clear(signal,mask); }

static inline int qurt_signal2_wait_cancellable(qurt_signal2_t *signal,
                                  unsigned int mask, 
                                  unsigned int attribute,
                                  unsigned int *p_returnmask) { return qurt_signal_wait_cancellable(signal,mask,attribute,p_returnmask); }

#endif /* QURT_SIGNAL2_H */
