/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_ERROR_H
#define H2_ERROR_H 1

/** @file h2_error.h
 @brief Basic Error Handling
*/
/** @addtogroup h2 
@{ */

/**
Sets up basic error handling, causing any exception to crash with a (hopefully)
useful diagnostic message

@param[in] exit_flag  TRUE == call exit()
@returns None
@dependencies None
*/

void h2_handle_errors(int exit_flag);

/**
If H2 error handling is enabled using h2_handle_errors(), this routine allows
the caller to substitute different handling for the specified event.

@param[in] eventnum	Event number to handle
@param[in] fn		Function pointer to event handler
@returns None
@dependencies None
*/
void h2_set_handler(int eventnum, void (*fn)(int));

/** @} */

#endif

