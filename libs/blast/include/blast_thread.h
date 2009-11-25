/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * BLAST thread.h
 * 
 * Create a thread
 */

#ifndef BLAST_THREAD_H
#define BLAST_THREAD_H 1

int blast_thread_create(void *pc, void *stack, void *arg, unsigned int prio, unsigned int asid);
void blast_thread_stop();
int blast_thread_myid();
void blast_yield();

#endif

