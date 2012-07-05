/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_FUTEX_PI_H
#define H2K_FUTEX_PI_H 1

#ifndef ASM
#include <context.h>
s32_t H2K_futex_lock_pi(u32_t *lock, H2K_thread_context *me) IN_SECTION(".text.core.futex");
s32_t H2K_futex_unlock_pi(u32_t *lock, H2K_thread_context *me) IN_SECTION(".text.core.futex");
#endif

#endif
