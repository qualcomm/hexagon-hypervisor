/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * H2 thread.h
 * 
 * Create a thread
 */

#ifndef H2_THREAD_H
#define H2_THREAD_H 1

/* trap0 handler forces vmblock to NULL, so no need to declare it here */
int h2_thread_create(void *pc, void *stack, void *arg, unsigned int prio);
void h2_thread_stop(void);
int h2_thread_myid(void);
void h2_yield(void);

void h2_thread_set_tid(unsigned int);
unsigned int h2_thread_get_tid (void);
#endif

