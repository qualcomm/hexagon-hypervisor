/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <h2_common_linear.h>
#include <h2_common_pmap.h>
#include <stdio.h>
#include <stdlib.h>

#define IDENTITY_SIZE SIZE_64K
#define IDENTITY_ENTRIES 0x420
#define ALIAS_A 0x40000000
#define ALIAS_B 0x40001000
#define PATTERN_A 0xa5a5a5a5
#define PATTERN_B 0x5a5a5a5a

/*
 * Keep the data pages and stage-1 table at nonzero 4K offsets inside one
 * 64K block.  Booter maps that block in stage 2 with --page_size 2.
 */
static struct {
	char padding[0x1000];
	volatile h2_u32_t data_a[0x1000 / sizeof(h2_u32_t)];
	volatile h2_u32_t data_b[0x1000 / sizeof(h2_u32_t)];
	H2K_linear_fmt_t table[IDENTITY_ENTRIES + 3];
} nested_map __attribute__((aligned(0x10000)));

static void FAIL(const char *message)
{
	printf("FAIL: %s\n", message);
	exit(1);
}

static H2K_linear_fmt_t make_entry(h2_u32_t vpn, h2_u32_t ppn, h2_u32_t size)
{
	H2K_linear_fmt_t entry;

	entry.raw = 0;
	entry.vpn = vpn;
	entry.ppn = ppn;
	entry.size = size;
	entry.cccc = L1WB_L2C;
	entry.xwru = URWX;
	return entry;
}

static int in_identity_map(const void *ptr)
{
	h2_u32_t addr = (h2_u32_t)ptr;

	return addr >= H2K_GUEST_START && addr < H2K_GUEST_START + 0x01000000;
}

int main()
{
	volatile h2_u32_t *alias_a = (volatile h2_u32_t *)ALIAS_A;
	volatile h2_u32_t *alias_b = (volatile h2_u32_t *)ALIAS_B;
	h2_u32_t map_base = (h2_u32_t)&nested_map;
	h2_u32_t i;

	if (!in_identity_map(&nested_map)) FAIL("nested map is outside identity mapping");
	if (((map_base >> PAGE_BITS) & 0xf) != 0) FAIL("nested map is not 64K aligned");
	if ((((h2_u32_t)nested_map.table >> PAGE_BITS) & 0xf) == 0) FAIL("table lacks 64K block offset");
	if ((((h2_u32_t)nested_map.data_a >> PAGE_BITS) & 0xf) == 0) FAIL("data A lacks 64K block offset");
	if ((((h2_u32_t)nested_map.data_b >> PAGE_BITS) & 0xf) == 0) FAIL("data B lacks 64K block offset");

	nested_map.data_a[0] = PATTERN_A;
	nested_map.data_b[0] = PATTERN_B;

	/* Keep the executing image, heap, and stack mapped after replacing stage 1. */
	for (i = 0; i < IDENTITY_ENTRIES; i++) {
		h2_u32_t page = (H2K_GUEST_START >> PAGE_BITS)
			+ (i << (IDENTITY_SIZE * 2));

		nested_map.table[i] = make_entry(page, page, IDENTITY_SIZE);
	}
	nested_map.table[IDENTITY_ENTRIES] = make_entry(ALIAS_A >> PAGE_BITS,
			(h2_u32_t)nested_map.data_a >> PAGE_BITS, SIZE_4K);
	nested_map.table[IDENTITY_ENTRIES + 1] = make_entry(ALIAS_B >> PAGE_BITS,
			(h2_u32_t)nested_map.data_b >> PAGE_BITS, SIZE_4K);
	nested_map.table[IDENTITY_ENTRIES + 2].raw = 0;
	h2_cache_dcclean_range(&nested_map, sizeof(nested_map));

	if (h2_vmtrap_newmap(nested_map.table, H2K_ASID_TRANS_TYPE_LINEAR,
			H2K_ASID_TLB_INVALIDATE_TRUE) != 0) {
		FAIL("h2_vmtrap_newmap");
	}

	if (alias_a[0] != PATTERN_A) {
		printf("data A 0x%08x alias A 0x%08x expected 0x%08x\n",
			nested_map.data_a[0], alias_a[0], PATTERN_A);
		FAIL("alias A translation");
	}
	if (alias_b[0] != PATTERN_B) {
		printf("data B 0x%08x alias B 0x%08x expected 0x%08x\n",
			nested_map.data_b[0], alias_b[0], PATTERN_B);
		FAIL("alias B translation");
	}

	alias_a[1] = PATTERN_B;
	alias_b[1] = PATTERN_A;
	if (nested_map.data_a[1] != PATTERN_B) FAIL("alias A write");
	if (nested_map.data_b[1] != PATTERN_A) FAIL("alias B write");

	puts("TEST PASSED");
	exit(0);
}
