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
#include <time.h>
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

/* 13 */
void H2K_vmtrap_cachectl(H2K_thread_context *me)
{
	/* FIXME: doing this like minivm for now */

	cacheop_type op = (cacheop_type)me->r00;
	// u32_t va = me->r01;
	// u32_t count = me->r02;
	u32_t i, j;
	u32_t idx = 0;

	if (me->r00 >= H2K_CACHECTL_BADOP) {
		me->r00 = -1;
		return;
	}

	me->r00 = 0;

	switch(op) {
	case H2K_CACHECTL_ICKILL:
	case H2K_CACHECTL_ICINVA:
		asm volatile
			(
			 " ickill \n"
			 );
		return;

	case H2K_CACHECTL_DCKILL:
	case H2K_CACHECTL_DCCLEANINVA:
		for (i = 0; i < CACHEIDX_MAX; i++) {
			asm volatile
				(
				 "dccleaninvidx(%0) \n"
				 :
				 : "r"(i)
				 );
		};
		return;

	case H2K_CACHECTL_IDSYNC:
		for (i = 0; i < WAYS_MAX; i++) {
			for (j = 0; j < (SETS_MAX * 32); j += 32) {
				idx += i + j;
				asm volatile
					(
					 "icinvidx(%0) \n"
					 "dccleanidx(%0) \n"
					 :
					 : "r"(idx)
					 );
			}
		}
		return;

	case H2K_CACHECTL_L2KILL:
	case H2K_CACHECTL_BADOP:  // shut up warning
		return;
	}
}

