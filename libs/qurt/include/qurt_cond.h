/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_COND_H
#define QURT_COND_H 
/**
  @file qurt_cond.h
  @brief Prototypes of Kernel condition variable object  API functions      

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

#include <h2.h>
#include <qurt_mutex.h>
#include <qurt_rmutex2.h>

/** @addtogroup condition_variables_types
@{ */

/** QuRT condition variable type.  */
typedef h2_cond_t qurt_cond_t;

/** @} */ /* end_addtogroup condition_variables_types */

 

static inline void qurt_cond_init(qurt_cond_t *cond) { h2_cond_init(cond); }

static inline void qurt_cond_destroy(qurt_cond_t *cond) {}

static inline void qurt_cond_signal(qurt_cond_t *cond) { h2_cond_signal(cond); }

static inline void qurt_cond_broadcast(qurt_cond_t *cond) { h2_cond_broadcast(cond); }

static inline void qurt_cond_wait(qurt_cond_t *cond, qurt_mutex_t *mutex)
{
	h2_cond_wait_rmutex(cond,mutex);
}

static inline void qurt_cond_wait2(qurt_cond_t *cond, qurt_rmutex2_t *mutex)
{
	return h2_cond_wait_rmutex(cond,mutex);
}

#endif /* QURT_COND_H */

