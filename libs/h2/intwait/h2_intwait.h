/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_INTWAIT_H
#define H2_INTWAIT_H 1

/** @file h2_intwait.h
 @brief Wait for an interrupt event
*/
/** @addtogroup h2 
@{ */

/**
Wait for an interrupt.  Blocks until the interrupt arrives.
@param[in] intwait	Interrupt to wait for
@returns Interrupt number on success, negative value on failure
@dependencies None
*/

int h2_intwait(unsigned int interrupt);

/** @} */

#endif

