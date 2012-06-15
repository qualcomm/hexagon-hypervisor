/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VMCACHE_H
#define H2K_VMCACHE_H 1

#include <c_std.h>
#include <context.h>

typedef enum {
	H2K_CACHECTL_ICKILL,
	H2K_CACHECTL_DCKILL,
	H2K_CACHECTL_L2KILL,
	H2K_CACHECTL_DCCLEANINVA,
	H2K_CACHECTL_ICINVA,
	H2K_CACHECTL_IDSYNC,
	H2K_CACHECTL_BADOP
} cacheop_type;

void H2K_vmtrap_cachectl(H2K_thread_context *me) IN_SECTION(".text.vm.funcs");

#endif

