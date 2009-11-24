/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLASTK_FUTEX_H
#define BLASTK_FUTEX_H 1

#define FUTEX_HASHBITS 6
#define FUTEX_HASHSIZE (1<<FUTEX_HASHBITS)

s32_t BLASTK_futex_wait(u32_t *lock, u32_t val, BLASTK_thread_context *me);
u32_t BLASTK_futex_resume(u32_t *lock, u32_t n_to_wake, BLASTK_thread_context *me);
void BLASTK_futex_init();

#endif
