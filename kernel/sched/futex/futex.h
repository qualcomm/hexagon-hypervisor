/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_FUTEX_H
#define H2K_FUTEX_H 1

#define FUTEX_HASHBITS 5
#define FUTEX_HASHSIZE (1<<FUTEX_HASHBITS)

s32_t H2K_futex_wait(u32_t *lock, u32_t val, H2K_thread_context *me);
u32_t H2K_futex_resume(u32_t *lock, u32_t n_to_wake, H2K_thread_context *me);
void H2K_futex_init();

#endif
