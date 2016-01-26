/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <cputime.h>
#include <globals.h>
#include <hw.h>

u64_t H2K_pcycles_get(H2K_thread_context *me)
{
	return H2K_get_pcycle_reg();
}

u64_t H2K_cputime_get(H2K_thread_context *me)
{
	u64_t now = H2K_pcycles_get(me);
	return me->totalcycles + (now - H2K_gp->oncpu_start[get_hwtnum()]);
}

