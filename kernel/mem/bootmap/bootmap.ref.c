/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <bootmap.h>
#include <bootmap_macros.h>
#include <linear.h>

/* FIXME: These shouldn't both use bootmap.def.  There should be distinct maps
	 for h2 boot and for the boot thread, from which all other threads inherit by
	 default.  Actually, there shouldn't be a boot "map" at all --- at start we
	 should create the necessary pinned global TLB entry for the segment where h2
	 lives. */

#define MEMORY_MAP(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) MEMORY_MAP_BOOT(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN)

u64_t H2K_bootmap[] IN_SECTION(".data.init.bootmap") = {
#include "bootmap.def"
	0ULL,
};

#undef MEMORY_MAP
#undef TLB_INVALID_ENTRY

#define MEMORY_MAP(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) MEMORY_MAP_THREAD(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN)

#define TLB_INVALID_ENTRY

H2K_linear_fmt_t H2K_linear_bootmap[] IN_SECTION(".data.init.bootmap") = {
#include "bootmap.def"
	{ .raw = 0 },
};

u64_t *H2K_bootmap_ptr = H2K_bootmap;

