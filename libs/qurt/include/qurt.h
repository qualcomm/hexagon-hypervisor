/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_H
#define QURT_H 

/**
  @file qurt.h 
  @brief  Contains kernel header files which provides kernel OS API functions, constants, and 
  definitions 

 EXTERNALIZED FUNCTIONS
  none

 INITIALIZATION AND SEQUENCING REQUIREMENTS
  none

 Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
 ======================================================================*/

 

#ifdef __cplusplus
extern "C" {
#endif

void qurt_init();

#include "qurt_prelim.h"
#include "qurt_consts.h"
#include "qurt_consts_internal.h"
#include "qurt_alloc.h"
#include "qurt_futex.h"
#include "qurt_mutex.h"
#include "qurt_pgalloc.h"
#include "qurt_pipe.h"
#include "qurt_printf.h"
#include "qurt_assert.h"
#include "qurt_thread.h"
#include "qurt_trace.h"
#include "qurt_cycles.h"
#include "qurt_sem.h"
#include "qurt_cond.h"
#include "qurt_barrier.h"
#include "qurt_fastint.h"
#include "qurt_allsignal.h"
#include "qurt_anysignal.h"
#include "qurt_signal.h"
#include "qurt_rmutex.h"
#include "qurt_pimutex.h"
#include "qurt_signal2.h"
#include "qurt_rmutex2.h"
#include "qurt_pimutex2.h"
#include "qurt_int.h"
#include "qurt_lifo.h"
#include "qurt_power.h"
#include "qurt_event.h"
#include "qurt_pmu.h"
//#include "qurt_version.h"
#include "qurt_tlb.h"
#include "qurt_memory.h"
#include "qurt_qdi.h"
#include "qurt_sclk.h"
#include "qurt_space.h"
#include "qurt_process.h"
#include "qurt_shmem.h"
#include "qurt_timer.h"
#include "qurt_tls.h"
#include "qurt_thread_context.h"
#include "qurt_hvx.h"
#include "qurt_island.h"
#include "qurt_utcb.h"

#ifdef __cplusplus
} /* closing brace for extern "C" */
#endif

#endif /* QURT_H */

