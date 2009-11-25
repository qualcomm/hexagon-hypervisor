/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast_futex.h>
#include <blast_mutex.h>
#include <blast_pipe.h>
#include <blast_alloc.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#define MAX_THREADS_TO_WAKE 32767

#if 0
static inline int atomic_circ_increment(unsigned int *val, unsigned int size, unsigned int recvidx)
{
	unsigned int ret;
	unsigned int newval;
	asm ("\n"
	"\t %0 = memw_locked(%2)\n"
	"\t { p0 = cmp.gt(%3,%0) // Is size-1 greater than me?\n"
	"\t   if (!p0.new) %1 = #0   // Nope, reset to zero\n"
	"\t   if (p0.new) %1 = add(%0,#1) } // Yes, OK to increment\n"
	"\t { p0 = cmp.eq(%4,%1) // Is the new val the recvidx?\n"
	"\t   if (p0.new) jump:nt 1f // yes, may be full, \n"
	"\t   if (p0.new) %0 = #-2 } // and mark as invalid\n"
	"\t memw_locked(%2,p0) = %1\n"
	"\t1: if (!p0) %0 = #-1\n" : "=&r"(ret),"=&r"(newval) :"r"(val),"r"(size-1),"r"(recvidx) : "p0","memory");
	return ret;
}
#endif

static inline blast_pipe_data_t get_recv_data(unsigned int *recvptr, blast_pipe_data_t *buf, unsigned int size) {
	blast_pipe_data_t ret;
	asm("\n"
	"\t1: r9 = memw_locked(%1)\n"
	"\t { p0 = cmp.gt(%2,r9) // is size-1 greater than me?\n"
	"\t   if (!p0.new) r9 = #0 // nope, reset to zero\n"
	"\t   if (p0.new) r9 = add(r9,#1) // yes, OK to increment\n"
	"\t   r8 = addasl(%3,r9,#3) } // form ptr to element\n"
	"\t %0 = memd(r8)\n"
	"\t memw_locked(%1,p0) = r9\n" 
	"\t if (!p0) jump 1b\n" 
		:"=&r"(ret) 
		:"r"(recvptr),"r"(size-1),"r"(buf)
		:"r8","r9","p0","memory");
	return ret;
}

blast_pipe_t * blast_pipe_alloc(unsigned int size)
{
	blast_pipe_t *ret;
	unsigned int datasize = ((size & (-sizeof(blast_pipe_data_t)))-sizeof(blast_pipe_t));
	if (sizeof(blast_pipe_t) + sizeof(blast_pipe_data_t) > size) return NULL;
	ret = blast_malloc(sizeof(blast_pipe_t));
	if (ret == NULL) return NULL;
	ret->data = blast_malloc(datasize);
	if (ret->data == NULL) {
		blast_free(ret);
		return NULL;
	}
	blast_sem_init_val(&ret->howfull, 0);
	ret->size = datasize/sizeof(blast_pipe_data_t);
	blast_sem_init_val(&ret->howempty, ret->size-1);
	ret->sendidx = 0;
	ret->recvidx = 0;
	return ret;
}

blast_pipe_t * blast_pipe_create(blast_pipe_t *pipe, blast_pipe_data_t *data, int data_elements)
{
	if (data_elements <= 0) return NULL;
	if (pipe == NULL) return NULL;
	if (data == NULL) return NULL;
	pipe->size = data_elements;
	blast_sem_init_val(&pipe->howfull,0);
	blast_sem_init_val(&pipe->howempty,data_elements-1);
	pipe->sendidx = 0;
	pipe->recvidx = 0;
	pipe->data = data;
	return pipe;
}

void blast_pipe_free(blast_pipe_t *pipe)
{
	blast_free(pipe->data);
	blast_free(pipe);
}

void blast_pipe_send(blast_pipe_t *pipe, blast_pipe_data_t data)
{
	int oldidx;
	unsigned int size = pipe->size;
	blast_sem_down(&pipe->howempty);
	blast_mutex_lock(&pipe->sendmutex);
	oldidx = pipe->sendidx;
	pipe->sendidx = ((oldidx == size) ? 0 : oldidx+1);
	pipe->data[oldidx] = data;
	blast_mutex_unlock(&pipe->sendmutex);
	blast_sem_up(&pipe->howfull);
}

int blast_pipe_trysend(blast_pipe_t *pipe, blast_pipe_data_t data)
{
	int oldidx;
	unsigned int size = pipe->size;
	if (blast_mutex_trylock(&pipe->sendmutex) != 0) return 0;
	if (blast_sem_trydown(&pipe->howempty) != 0) {
		blast_mutex_unlock(&pipe->sendmutex);
		return 0;
	}
	oldidx = pipe->sendidx;
	pipe->sendidx = ((oldidx == size) ? 0 : oldidx+1);
	pipe->data[oldidx] = data;
	blast_mutex_unlock(&pipe->sendmutex);
	blast_sem_up(&pipe->howfull);
	return 1;
}

blast_pipe_data_t blast_pipe_recv(blast_pipe_t *pipe)
{
	blast_pipe_data_t data;
	unsigned int size = pipe->size;
	blast_sem_down(&pipe->howfull);
	data = get_recv_data(&pipe->recvidx,pipe->data,size);
	blast_sem_up(&pipe->howempty);
	return data;
}

blast_pipe_data_t blast_pipe_tryrecv(blast_pipe_t *pipe, int *success)
{
	blast_pipe_data_t data;
	unsigned int size = pipe->size;
	if (blast_sem_trydown(&pipe->howfull) != 0) {
		*success = 0;
		return 0;
	}
	data = get_recv_data(&pipe->recvidx,pipe->data,size);
	blast_sem_up(&pipe->howempty);
	*success = 1;
	return data;
}

