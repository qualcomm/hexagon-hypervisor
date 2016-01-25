/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_PIPE_H
#define H2_PIPE_H 1

/** @file h2_pipe.h
 @brief Simple datagram FIFO message queue
*/
/** @addtogroup h2 
@{ */

#include <h2_sem.h>

/**
The data element is a 64-bit value
*/

typedef unsigned long long int h2_pipe_data_t;

/**
@brief Structure for a pipe.  Do not use directly.
*/
typedef struct {
	h2_sem_t howfull;
	h2_sem_t howempty;
	unsigned int sendidx;
	unsigned int recvidx;
	unsigned int size;
	h2_mutex_t sendmutex;
	h2_pipe_data_t *data;
} h2_pipe_t;

/**
Allocate a pipe, with specified total allocation size.
@note This calls malloc internally.  To specify a memory location, use h2_pipe_create
@param[in] size_in_bytes	Number of bytes to allocate for the pipe
@returns Pointer to the pipe, or NULL on failure
@dependencies None
*/

h2_pipe_t * h2_pipe_alloc(unsigned int size_in_bytes);

/**
Initialize a pre-allocated pipe.
@param[in] pipe		Address of the pipe object
@param[in] data		Address of the data array
@param[in] data_elements	Number of elements in the data array
@returns Pointer to the pipe, or NULL on failure
@dependencies None
*/

h2_pipe_t * h2_pipe_create(h2_pipe_t *pipe, h2_pipe_data_t *data, int data_elements);

/**
Free a pipe created with h2_pipe_alloc
@param[in] pipe		Address of the pipe object
@returns None
@dependencies None
*/
void h2_pipe_free(h2_pipe_t *pipe);

/**
Send data through the pipe
@note If the pipe is full, this will block until data is available
@param[in] pipe		Address of the pipe object
@param[in] data		Data to send
@returns None
@dependencies None
*/
void h2_pipe_send(h2_pipe_t *pipe, h2_pipe_data_t data);

/**
Receive data from the pipe
@note If the pipe is empty, this will block until data is available
@param[in] pipe		Address of the pipe object
@returns Data received from the pipe
@dependencies None
*/
h2_pipe_data_t h2_pipe_recv(h2_pipe_t *pipe);

/**
Attempt to send data through the pipe.  If the pipe is full, return failure
@param[in] pipe		Address of the pipe object
@param[in] data		Data to send
@returns 1 on success, zero on failure
@dependencies None
*/
int h2_pipe_trysend(h2_pipe_t *pipe, h2_pipe_data_t data);

/**
Attempt to receive data from the pipe.  If the pipe is empty, return failure
@note This calls malloc internally.  To specify a memory location, use h2_pipe_create
@param[in] size_in_bytes	Number of bytes to allocate for the pipe
@param[out] success		Address of the word to modify with success/failure indication
@returns The data received from the pipe if *success is set to 1, invalid if *success is set to 0.
@dependencies None
*/
h2_pipe_data_t h2_pipe_tryrecv(h2_pipe_t *pipe, int *success);

/** @} */

#endif

