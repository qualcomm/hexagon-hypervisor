/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_FUTEX_CLASSIC_H
#define H2K_FUTEX_CLASSIC_H 1

#ifndef ASM
#include <context.h>

s32_t H2K_futex_wait(u32_t *lock, u32_t val, H2K_thread_context *me) IN_SECTION(".text.core.futex");
s32_t H2K_futex_resume(u32_t *lock, u32_t n_to_wake, H2K_thread_context *me) IN_SECTION(".text.core.futex");
#endif

#endif
