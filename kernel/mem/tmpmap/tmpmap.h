/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TMPMAP_H
#define H2K_TMPMAP_H 1

#include <c_std.h>

u32_t H2K_tmpmap_add_and_lock(pa_t pa, u32_t cccc) IN_SECTION(".text.mem.tmpmap");

void H2K_tmpmap_remove_and_unlock(void) IN_SECTION(".text.mem.tmpmap");

void H2K_tmpmap_init(void) IN_SECTION(".text.mem.tmpmap");

#endif
