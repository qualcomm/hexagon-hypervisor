/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_PRIO_H
#define H2_PRIO_H 1

/** @file h2_prio.h
 @brief Mutexes allow at most one thread to hold the mutex at a time
*/
/** @addtogroup h2 
@{ */

/**
Get the current base priority.
@param[in] threadid	Thread ID for thread to get priority
@returns Current priority
@dependencies None
*/
int h2_get_prio(unsigned int threadid);

/**
Set the priority for a thread.
@param[in] threadid	Thread ID for thread to change priority
@param[in] newprio	New priority for the thread
@returns Zero on success, nonzero on failure
@dependencies None
@note Not currently implemented.
*/
int h2_set_prio(unsigned int threadid, unsigned int newprio);

/** @} */

#endif

