/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_FASTINT_H
#define H2_FASTINT_H 1

/** @file h2_fastint.h
 @brief Fast Interrupt Configuration
*/
/** @addtogroup h2 
@{ */

/**
Register a fast interrupt handler
@param[in] intno	Interrupt number to handle
@param[in] fn		Fast interrupt handler
@returns None
@dependencies None
*/

void h2_register_fastint(int intno, int (*fn)(int));

/**
Unregister a fast interrupt handler
@param[in] intno	Interrupt number to no longer handle
@returns None
@dependencies None
*/

void h2_deregister_fastint(int intno);

/** @} */

#endif
