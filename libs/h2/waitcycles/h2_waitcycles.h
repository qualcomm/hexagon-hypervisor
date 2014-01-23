/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_WAITCYCLES_H
#define H2_WAITCYCLES_H 1

/** @file h2_waitcycles.h
 @brief Query the number of cycles spent in wait mode
*/
/** @addtogroup h2 
@{ */

/**
Get waitcycles.
@param[i] htid  Hardware thread ID
@returns Total cycles spent in wait mode
@dependencies None
*/
unsigned long long int h2_waitcycles(unsigned long htid);

#endif

