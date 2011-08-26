/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <bootmap.h>
#include <bootmap_macros.h>
#include <linear.h>

#define MEMORY_MAP(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) MEMORY_MAP_BOOT(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN)

u64_t H2K_bootmap[] IN_SECTION(".data.init.bootmap") = {
#include "bootmap.def"
	0ULL,
};

#undef MEMORY_MAP

#define MEMORY_MAP(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN) MEMORY_MAP_THREAD(G,ASID,VPN,PERM,CFIELD,PGSIZE,MAINAUX,PPN)

H2K_linear_fmt_t H2K_linear_bootmap[] IN_SECTION(".data.init.bootmap") = {
#include "bootmap.def"
	{ .raw = 0 },
};

u64_t *H2K_bootmap_ptr = H2K_bootmap;

