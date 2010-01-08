/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <time.h>

u64_t H2K_pcycles_get(H2K_thread_context *me)
{
	u64_t ret;
	asm (
	"1: %H0 = pcyclehi \n"
	" %L0 = pcyclelo \n"
	" r15 = pcyclehi \n"
	" p0 = cmp.eq(r15,%H0) \n"
	" if (!p0) jump 1b \n" : "=r"(ret) : : "r15","p0");
	return ret;
}

u64_t H2K_cputime_get(H2K_thread_context *me)
{
	u64_t now = H2K_pcycles_get(me);
	return me->totalcycles + (now - me->oncpu_start);
}

