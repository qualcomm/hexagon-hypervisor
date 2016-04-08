/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_FUTEX_H
#define QURT_FUTEX_H
/**
  @file  qurt_futex.h

  @brief  Prototypes of Kernel futex API functions      
  Futex calls directly in case you want to play with them from C
  
 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

#include <h2.h>
#include "qurt_prelim.h"

static inline int qurt_futex_wait(void *lock, int val) { return h2_futex_wait(lock,val); }
static inline int qurt_futex_wait_cancellable(void *lock, int val) { return qurt_futex_wait(lock,val); }
//static inline int qurt_futex_wait64(void *lock, long long val) { R_UNSUPPORTED; }
static inline int qurt_futex_wake(void *lock, int n_to_wake) { return h2_futex_wake(lock,n_to_wake); }

#endif /* QURT_FUTEX_H */

