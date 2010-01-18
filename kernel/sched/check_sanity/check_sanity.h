/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_CHECK_SANITY_H
#define H2K_CHECK_SANITY_H 1

#include <c_std.h>

void H2K_check_sched_mask();
u64_t H2K_check_sanity(const u64_t retval);
u64_t H2K_check_sanity_unlock(const u64_t retval);

#ifdef DEBUG
#include <checker_kernel_locked.h>
#include <check_sanity_debug.h>
#endif

#endif
