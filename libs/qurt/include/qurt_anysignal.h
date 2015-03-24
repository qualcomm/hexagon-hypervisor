/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_ANYSIGNAL_H
#define QURT_ANYSIGNAL_H 
/**
  @file qurt_anysignal.h
  @brief  Prototypes of Kernel signal  API functions      

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

#include <qurt_signal.h>

/**@ingroup anysignals_types                                                 
 The caller of signal_wait can be woken up if any bits in mask are set.  */
typedef h2_anysignal_t qurt_anysignal_t;

 

static inline void qurt_anysignal_init(qurt_anysignal_t *signal)
{
  h2_anysignal_init(signal);
}

static inline void qurt_anysignal_destroy(qurt_anysignal_t *signal) { }

static inline unsigned int qurt_anysignal_wait(qurt_anysignal_t *signal, unsigned int mask)
{
  return h2_anysignal_wait(signal, mask);
}

static inline unsigned int qurt_anysignal_set(qurt_anysignal_t *signal, unsigned int mask)
{
	return h2_anysignal_set(signal,mask);
}

static inline unsigned int qurt_anysignal_get(qurt_anysignal_t *signal)
{
	return signal->signals;
}

static inline unsigned int qurt_anysignal_clear(qurt_anysignal_t *signal, unsigned int mask)
{
	return h2_anysignal_clear(signal,mask);
}

#endif /* QURT_ANYSIGNAL_H */
