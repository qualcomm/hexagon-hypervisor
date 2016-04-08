/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_LINEAR_H
#define H2K_LINEAR_H 1

#include <c_std.h>
#include <context.h>
#include <tlbfmt.h>
#include <max.h>
#include <translate.h>
#include <h2_common_linear.h>

H2K_translation_t H2K_linear_translate(H2K_translation_t in, H2K_asid_entry_t info) IN_SECTION(".text.mem.linear");

#endif

