/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_BOOTMAP_H
#define H2K_BOOTMAP_H 1

#include <c_std.h>
#include <linear.h>

extern u64_t *H2K_bootmap_ptr IN_SECTION(".data.bootmap");
extern H2K_linear_fmt_t H2K_linear_bootmap[] IN_SECTION(".data.bootmap");

#endif

