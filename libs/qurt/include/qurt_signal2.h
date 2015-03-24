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

#define QURT_SIGNAL_ATTR_WAIT_ANY 0x00000000
#define QURT_SIGNAL_ATTR_WAIT_ALL 0x00000001

/** @addtogroup signals_types
@{ */
/** qurt_signal2 type                                           
 */
typedef struct {
   /** @cond */
   unsigned int cur_mask __attribute__((aligned(8)));  /* Current set of signal bits that are set */
   unsigned int sig_state;                             /* Current state */
                                                       /*   Bit 0 = currently in "anysignal" wait */
                                                       /*   Bit 1 = currently in "allsignal" wait */
                                                       /*   Bit 2 = currently in "interrupt" wait */
                                                       /*   Bits 31-3 = reference count field */
   unsigned int queue;                                 /* Kernel-maintained "futex queue" value */
   unsigned int wait_mask;                             /* If sig_state indicates a waiter is present, this is the wait mask */
   /** @endcond */
} qurt_signal2_t;
/** @} */ /* end_addtogroup signals_types */

 

void qurt_signal2_init(qurt_signal2_t *signal);

void qurt_signal2_destroy(qurt_signal2_t *signal);

unsigned int qurt_signal2_wait(qurt_signal2_t *signal, unsigned int mask, 
                unsigned int attribute);

static inline unsigned int qurt_signal2_wait_any(qurt_signal2_t *signal, unsigned int mask)
{
  return qurt_signal2_wait(signal, mask, QURT_SIGNAL_ATTR_WAIT_ANY);
}

static inline unsigned int qurt_signal2_wait_all(qurt_signal2_t *signal, unsigned int mask)
{
  return qurt_signal2_wait(signal, mask, QURT_SIGNAL_ATTR_WAIT_ALL);
}

void qurt_signal2_set(qurt_signal2_t *signal, unsigned int mask);

unsigned int qurt_signal2_get(qurt_signal2_t *signal);

void qurt_signal2_clear(qurt_signal2_t *signal, unsigned int mask);

int qurt_signal2_wait_cancellable(qurt_signal2_t *signal,
                                  unsigned int mask, 
                                  unsigned int attribute,
                                  unsigned int *p_returnmask);

#endif /* QURT_SIGNAL2_H */
