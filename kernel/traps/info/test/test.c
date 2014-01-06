/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <info.h>
#include <stdlib.h>
#include <stdio.h>
#include <globals.h>
#include <stlb.h>
#include <alloc.h>

#define UNIT_BYTES (ALLOC_UNIT * sizeof(H2K_mem_alloc_tag_t))

H2K_mem_alloc_tag_t Heap[H2K_ALLOC_HEAP_SIZE] __attribute__((aligned(ALLOC_UNIT))) = {{{.size = 0, .free = 0}}};

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

int main() {

	info_stlb_type stlb_info;

	u32_t val;

	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	H2K_kg_init(0, 0);
	H2K_mem_alloc_init(Heap, H2K_ALLOC_HEAP_SIZE);
	if (H2K_mem_stlb_alloc() == -1) FAIL("STLB alloc");

	if (H2K_trap_info(INFO_BUILD_ID, 0) != H2K_GIT_COMMIT) FAIL("Build ID");

#ifdef H2K_USE_TCM
	if ((H2K_trap_info(INFO_BOOT_FLAGS, 0) & 1) != 1) FAIL("USE_TCM");
#else
	if ((H2K_trap_info(INFO_BOOT_FLAGS, 0) & 1) != 0) FAIL("USE_TCM");
#endif

	stlb_info.raw = H2K_trap_info(INFO_STLB, 0);
	if (stlb_info.stlb_max_sets_log2 != STLB_MAX_SETS_LOG2) FAIL("STLB sets");
	if (stlb_info.stlb_max_ways != STLB_MAX_WAYS) FAIL("STLB ways");
	if (stlb_info.stlb_size != STLB_MULT) FAIL("STLB size");
	if (stlb_info.stlb_enabled != 1) FAIL("STLB enabled");

	asm volatile ( "%0 = syscfg\n" : "=r" (val));
	if (H2K_trap_info(INFO_SYSCFG, 0) != val) FAIL("SYSCFG");

	asm volatile ( "%0 = rev\n" : "=r" (val));
	if (H2K_trap_info(INFO_REV, 0) != val) FAIL("REV");

	puts("TEST PASSED\n");
	return 0;
}
