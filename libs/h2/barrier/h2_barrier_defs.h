/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_BARRIER__DEFS_H
#define H2_BARRIER_DEFS_H 1

/** @file h2_barrier_defs.h
 @brief Barrier waits for a certain number of threads to reach the same point before continuing
*/
/** @addtogroup h2 
@{ */

/** Return value for one of the threads at the barrier, arbitrarily */
#define H2_BARRIER_SERIAL_THREAD 1

/** Return value for all other threads at the barrier */
#define H2_BARRIER_OTHER 0

#endif
