/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * H2 mutex.h
 * 
 * Futex calls directly in case you want to play with
 * them from C
 */

#ifndef H2_FUTEX_H
#define H2_FUTEX_H 1

int h2_futex_wait(void *lock, int val);
int h2_futex_wake(void *lock, int n_to_wake);
int h2_futex_lock_pi(void *lock);
int h2_futex_unlock_pi(void *lock);

#endif

