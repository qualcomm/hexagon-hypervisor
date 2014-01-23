/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_THREAD_H
#define H2_THREAD_H 1

/** @file h2_thread.h
 @brief Manage threads
*/
/** @addtogroup h2 
@{ */

/* trap0 handler forces vmblock to NULL, so no need to declare it here */

/**
Create a thread.
@param[in] pc		Address of the first instruction to execute
@param[in] stack	Address of the start of the stack
@param[in] arg		Arbitrary argument to pass in r0 to the created thread
@param[in] prio		Priority to create the thread at
@returns ID of the created thread, or odd value on failure.
@dependencies None
*/
int h2_thread_create(void *pc, void *stack, void *arg, unsigned int prio);

/**
Terminate the current thread with the given status.
@param[in] status  Termination status value
@returns None; Does not return.
@dependencies None
*/
void h2_thread_stop(int status);

/**
Obtain the ID of the calling thread
@returns ID of the calling thread
@dependencies None
*/
unsigned int h2_thread_myid(void);

/**
Ask the scheduler to yield to another thread at the same priority.
@returns None
@dependencies None
*/
void h2_yield(void);

/**
Set the Software TID, a value used by the debug hardware to distinguish software threads
@param[in] tid		TID value to use
@returns None
@dependencies None
*/
void h2_thread_set_tid(unsigned int tid);

/**
Read the Software TID, a value used by the debug hardware to distinguish software threads
@returns Current TID value
@dependencies None
*/
unsigned int h2_thread_get_tid(void);

/**
Read values from the given offset in the thread context specified by id.
@param[in] id  Thread ID to query
@param[in] offset  Byte offset of data to read
@returns 64-bit value at offset, or -1ULL if error
*/
unsigned long long int h2_thread_state(unsigned int id, unsigned int offset);

#endif

