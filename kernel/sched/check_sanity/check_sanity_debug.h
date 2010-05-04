/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*  This file is generated from the documentation!  Do not hand edit!  */
#include <assert.h>
static u64_t H2K_check_sanity_debug(const u64_t returnval) {
	const u64_t arg0 = returnval;
	u64_t retval;
	assert(checker_kernel_locked());
	retval = H2K_check_sanity(returnval);
	assert(checker_kernel_locked());
	assert(retval == arg0);
	return(retval);
}

static void H2K_check_sched_mask_debug() {
	assert(checker_kernel_locked());
	H2K_check_sched_mask();
}

static u64_t H2K_check_sanity_unlock_debug(const u64_t returnval) {
	const u64_t arg0 = returnval;
	u64_t retval;
	assert(checker_kernel_locked());
	retval = H2K_check_sanity_unlock(returnval);
	assert(!checker_kernel_locked());
	assert(retval == arg0);
	return(retval);
}

