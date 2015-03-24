/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_THREAD_CONTEXT_H
#define QURT_THREAD_CONTEXT_H
/**
  @file qurt_thread_context.h 
  @brief Kernel thread context structure
			
EXTERNAL FUNCTIONS

INITIALIZATION AND SEQUENCING REQUIREMENTS
   None.

Copyright (c) 2013  by Qualcomm Technologies, Inc.  All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

=============================================================================*/

#include <qurt_qdi_constants.h>

#define THREAD_ITERATOR_END ((qurt_thread_t)(-1))

static inline int qurt_thread_iterator_create(void)
{
	return 0;
}

static inline qurt_thread_t qurt_thread_iterator_next(int iter)
{
	return THREAD_ITERATOR_END;
}

static inline int qurt_thread_iterator_destroy(int iter)
{
	return 0;
}

int qurt_thread_context_get_tname(unsigned int thread_id, char *name, unsigned char max_len);
int qurt_thread_context_get_prio(unsigned int thread_id, unsigned char *prio);
int qurt_thread_context_get_pcycles(unsigned int thread_id, unsigned long long int *pcycles);
int qurt_thread_context_get_stack_base(unsigned int thread_id, unsigned int *sbase);
int qurt_thread_context_get_stack_size(unsigned int thread_id, unsigned int *ssize);

int qurt_thread_context_get_pid(unsigned int thread_id, unsigned int *pid);
int qurt_thread_context_get_pname(unsigned int thread_id, char *name, unsigned int len);

#endif
