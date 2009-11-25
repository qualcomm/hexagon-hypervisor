/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * BLAST mutex.h
 * 
 * Futex calls directly in case you want to play with
 * them from C
 */

#ifndef BLAST_FUTEX_H
#define BLAST_FUTEX_H 1

int blast_futex_wait(void *lock, int val);
int blast_futex_wake(void *lock, int n_to_wake);

#endif

