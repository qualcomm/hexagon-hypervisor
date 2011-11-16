/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <time.h>
#include <globals.h>
#include <hw.h>

u64_t H2K_pcycles_get(H2K_thread_context *me)
{
	/* EJP: FIXME: Move into hw.h, use pair xfer for v4 and later */
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
	/* FIXME: just return pcycles now, since linux needs this */
	u64_t now = H2K_pcycles_get(me);
	//return me->totalcycles + (now - H2K_gp->oncpu_start[get_hwtnum()]);
	return now;
}

