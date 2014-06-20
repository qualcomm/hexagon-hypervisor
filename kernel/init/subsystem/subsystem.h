/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_SUBSYSTEM_H
#define H2K_SUBSYSTEM_H 1

#include <c_std.h>
#include <max.h>

#ifdef HAVE_EXTENSIONS
void H2K_hvx_init(u32_t devpage_offset) IN_SECTION(".text.init.int");
#endif

#endif
