/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * H2 pipe interface
 * 
 * This is a pipe or message queue
 * It will block if too full (send) or empty (receive)
 * Unless you use a nonblocking option
 * All datagrams are 64 bits.
 */

#ifndef H2_PIPE_H
#define H2_PIPE_H 1

#include <h2_sem.h>

typedef unsigned long long int h2_pipe_data_t;

typedef struct {
	h2_sem_t howfull;
	h2_sem_t howempty;
	unsigned int sendidx;
	unsigned int recvidx;
	unsigned int size;
	h2_mutex_t sendmutex;
	h2_pipe_data_t *data;
} h2_pipe_t;

h2_pipe_t * h2_pipe_alloc(unsigned int size_in_bytes);
h2_pipe_t * h2_pipe_create(h2_pipe_t *pipe, h2_pipe_data_t *data, int data_elements);
void h2_pipe_free(h2_pipe_t *pipe);

void h2_pipe_send(h2_pipe_t *pipe, h2_pipe_data_t data);
h2_pipe_data_t h2_pipe_recv(h2_pipe_t *pipe);

int h2_pipe_trysend(h2_pipe_t *pipe, h2_pipe_data_t data);
h2_pipe_data_t h2_pipe_tryrecv(h2_pipe_t *pipe, int *success);

#endif

