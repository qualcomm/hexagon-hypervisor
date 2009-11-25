/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * BLAST pipe interface
 * 
 * This is a pipe or message queue
 * It will block if too full (send) or empty (receive)
 * Unless you use a nonblocking option
 * All datagrams are 64 bits.
 */

#ifndef BLAST_PIPE_H
#define BLAST_PIPE_H 1

#include <blast_sem.h>

typedef unsigned long long int blast_pipe_data_t;

typedef struct {
	blast_sem_t howfull;
	blast_sem_t howempty;
	unsigned int sendidx;
	unsigned int recvidx;
	unsigned int size;
	blast_mutex_t sendmutex;
	blast_pipe_data_t *data;
} blast_pipe_t;

blast_pipe_t * blast_pipe_alloc(unsigned int size_in_bytes);
blast_pipe_t * blast_pipe_create(blast_pipe_t *pipe, blast_pipe_data_t *data, int data_elements);
void blast_pipe_free(blast_pipe_t *pipe);

void blast_pipe_send(blast_pipe_t *pipe, blast_pipe_data_t data);
blast_pipe_data_t blast_pipe_recv(blast_pipe_t *pipe);

int blast_pipe_trysend(blast_pipe_t *pipe, blast_pipe_data_t data);
blast_pipe_data_t blast_pipe_tryrecv(blast_pipe_t *pipe, int *success);

#endif

