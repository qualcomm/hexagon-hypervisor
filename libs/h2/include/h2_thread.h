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

int h2_thread_create(void *pc, void *stack, void *arg, unsigned int prio, unsigned int trapmask);
void h2_thread_stop(void);
int h2_thread_myid(void);
void h2_yield(void);

#endif

