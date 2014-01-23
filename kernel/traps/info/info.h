/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_INFO_H
#define H2K_INFO_H 1

#include <h2_common_info.h>

u32_t H2K_trap_info(info_type op, H2K_thread_context *me) IN_SECTION(".text.misc.info");

#endif
