/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_FUTEX_MISC_H
#define H2K_FUTEX_MISC_H 1

#ifndef ASM
#include <context.h>
void H2K_futex_init() IN_SECTION(".text.init.futex");
void H2K_futex_cancel(H2K_thread_context *dst) IN_SECTION(".text.core.futex");
#endif

#endif

