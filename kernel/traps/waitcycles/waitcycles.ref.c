/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <max.h>
#include <globals.h>
#include <time.h>
#include <hw.h>
#include <waitcycles.h>

u64_t H2K_waitcycles_get(u32_t htid, H2K_thread_context *me)
{
	if (htid >= MAX_HTHREADS) return 0;
	if ((H2K_get_modectl() & (1 << (16 + htid))) == 0) return H2K_gp->waitcycles[htid];
	return H2K_gp->waitcycles[htid] + H2K_pcycles_get(me) - H2K_gp->oncpu_wait[htid];
}
