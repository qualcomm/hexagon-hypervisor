/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_SPINLOCK_H
#define H2K_SPINLOCK_H 1

#include <c_std.h>

typedef u32_t H2K_spinlock_t;

static inline void H2K_spinlock_init(H2K_spinlock_t *lock) { *lock = 0; }
void H2K_spinlock_lock(H2K_spinlock_t *lock);
int H2K_spinlock_trylock(H2K_spinlock_t *lock);
//static inline void H2K_spinlock_unlock(H2K_spinlock_t *lock) { *lock = 0; }
void H2K_spinlock_unlock(H2K_spinlock_t *lock);

#endif
