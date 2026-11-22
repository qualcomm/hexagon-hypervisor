/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_PRIO_H
#define H2_PRIO_H 1

/** @file h2_prio.h
 @brief Get or set the calling thread's base priority.
*/
/** @addtogroup h2 
@{ */

/**
Get the calling thread's base priority.
@returns Current priority
@dependencies None
*/
int h2_get_prio(void);

/**
Set the calling thread's base priority.
@param[in] newprio	New priority for the calling thread
@returns Previous priority, or -1 on failure
@dependencies None
*/
int h2_set_prio(unsigned int newprio);

/** @} */

#endif

