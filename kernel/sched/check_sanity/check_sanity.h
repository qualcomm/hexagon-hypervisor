/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef BLASTK_CHECK_SANITY_H
#define BLASTK_CHECK_SANITY_H 1

#include <c_std.h>
#ifdef DEBUG
#include <check_sanity_debug.h>
#endif

u64_t BLASTK_check_sanity(const u64_t retval);
u64_t BLASTK_check_sanity_unlock(const u64_t retval);

#endif
