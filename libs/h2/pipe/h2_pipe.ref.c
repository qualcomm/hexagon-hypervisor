/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_futex.h>
#include <h2_mutex.h>
#include <h2_pipe.h>
#include <h2_alloc.h>

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

static inline h2_pipe_data_t get_recv_data(unsigned int *recvptr, h2_pipe_data_t *buf, unsigned int size) {
	h2_pipe_data_t ret;
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

h2_pipe_t * h2_pipe_alloc(unsigned int size)
{
	h2_pipe_t *ret;
	unsigned int datasize = ((size & (-sizeof(h2_pipe_data_t)))-sizeof(h2_pipe_t));
	if (sizeof(h2_pipe_t) + sizeof(h2_pipe_data_t) > size) return NULL;
	ret = h2_malloc(sizeof(h2_pipe_t));
	if (ret == NULL) return NULL;
	ret->data = h2_malloc(datasize);
	if (ret->data == NULL) {
		h2_free(ret);
		return NULL;
	}
	h2_sem_init_val(&ret->howfull, 0);
	ret->size = datasize/sizeof(h2_pipe_data_t);
	h2_sem_init_val(&ret->howempty, ret->size);
	ret->sendidx = 0;
	ret->recvidx = 0;
	h2_mutex_init(&ret->sendmutex);
	return ret;
}

h2_pipe_t * h2_pipe_create(h2_pipe_t *pipe, h2_pipe_data_t *data, int data_elements)
{
	if (data_elements <= 0) return NULL;
	if (pipe == NULL) return NULL;
	if (data == NULL) {
		if ((data = h2_malloc(sizeof(*data) * (data_elements+1))) == NULL) return NULL;
	}
	pipe->size = data_elements;
	h2_sem_init_val(&pipe->howfull,0);
	h2_sem_init_val(&pipe->howempty,data_elements);
	pipe->sendidx = 0;
	pipe->recvidx = 0;
	pipe->data = data;
	h2_mutex_init(&pipe->sendmutex);
	return pipe;
}

void h2_pipe_free(h2_pipe_t *pipe)
{
	h2_free(pipe->data);
	h2_free(pipe);
}

void h2_pipe_send(h2_pipe_t *pipe, h2_pipe_data_t data)
{
	int oldidx;
	unsigned int size = pipe->size;
	h2_sem_down(&pipe->howempty);
	h2_mutex_lock(&pipe->sendmutex);
	oldidx = pipe->sendidx;
	pipe->sendidx = ((oldidx == (size-1)) ? 0 : oldidx+1);
	pipe->data[oldidx] = data;
	h2_mutex_unlock(&pipe->sendmutex);
	h2_sem_up(&pipe->howfull);
}

int h2_pipe_trysend(h2_pipe_t *pipe, h2_pipe_data_t data)
{
	int oldidx;
	unsigned int size = pipe->size;
	if (h2_mutex_trylock(&pipe->sendmutex) != 0) return 0;
	if (h2_sem_trydown(&pipe->howempty) != 0) {
		h2_mutex_unlock(&pipe->sendmutex);
		return 0;
	}
	oldidx = pipe->sendidx;
	pipe->sendidx = ((oldidx == (size-1)) ? 0 : oldidx+1);
	pipe->data[oldidx] = data;
	h2_mutex_unlock(&pipe->sendmutex);
	h2_sem_up(&pipe->howfull);
	return 1;
}

h2_pipe_data_t h2_pipe_recv(h2_pipe_t *pipe)
{
	h2_pipe_data_t data;
	unsigned int size = pipe->size;
	h2_sem_down(&pipe->howfull);
	data = get_recv_data(&pipe->recvidx,pipe->data,size);
	h2_sem_up(&pipe->howempty);
	return data;
}

h2_pipe_data_t h2_pipe_tryrecv(h2_pipe_t *pipe, int *success)
{
	h2_pipe_data_t data;
	unsigned int size = pipe->size;
	if (h2_sem_trydown(&pipe->howfull) != 0) {
		*success = 0;
		return 0;
	}
	data = get_recv_data(&pipe->recvidx,pipe->data,size);
	h2_sem_up(&pipe->howempty);
	*success = 1;
	return data;
}

