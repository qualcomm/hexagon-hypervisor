/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_STMODE_H
#define H2K_STMODE_H 1

#include <c_std.h>

s32_t H2K_stmode_begin() IN_SECTION(".text.misc.stmode");
void  H2K_stmode_end() IN_SECTION(".text.misc.stmode");

#endif

