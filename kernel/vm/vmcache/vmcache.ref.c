/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <vmcache.h>
#include <vmdefs.h>
#include <max.h>
#include <vmint.h>
#include <atomic.h>
#include <stop.h>
#include <yield.h>
#include <asid.h>
#include <cputime.h>
#include <stlb.h>
#include <tlbmisc.h>
#include <globals.h>
#include <hw.h>
#include <id.h>
#include <thread.h>
#include <create.h>
#include <runlist.h>
#include <fatal.h>
#include <vmint.h>
#include <dosched.h>
#include <vmwork.h>
#include <cache.h>

/* 13 */
void H2K_vmtrap_cachectl(H2K_thread_context *me)
{
	/* FIXME: doing this like minivm for now */

	cacheop_type op = (cacheop_type)me->r00;
	// u32_t va = me->r01;
	// u32_t count = me->r02;

	if (me->r00 >= H2K_CACHECTL_BADOP) {
		me->r00 = -1;
		return;
	}

	me->r00 = 0;

	switch(op) {
	case H2K_CACHECTL_ICKILL:
	case H2K_CACHECTL_ICINVA:
		H2K_ickill();
		return;

	case H2K_CACHECTL_DCKILL:
	case H2K_CACHECTL_DCCLEANINVA:
		H2K_cache_d_cleaninv();
		return;

	case H2K_CACHECTL_IDSYNC:
		H2K_cache_d_clean();
		H2K_cache_i_inv();
		return;

	case H2K_CACHECTL_L2KILL:
		H2K_cache_l2_cleaninv();
		return;
	case H2K_CACHECTL_BADOP:  // shut up warning
		return;
	}
}

