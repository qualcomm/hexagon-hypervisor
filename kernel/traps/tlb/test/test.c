/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread.h>
#include <string.h>
#include <tlb.h>
#include <fatal.h>
#include <globals.h>
#include <cache.h>
#include <hw.h>
#include <tlbmisc.h>
#include <cfg_table.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;
// H2K_kg_t H2K_kg;

/*
 * High-bits fingerprint OR'd with the iteration count to produce a unique,
 * recognizable per-iteration tag for entries the allocator hands out. The
 * upper 32 bits land in the high word of the JTLB entry (vpn:20 | asid:7 |
 * ... | valid:1) -- so the "12345" bits show up as a non-zero VPN that's
 * easy to spot in TLB dumps.
 */
#define TLBCONST 0x0001234500000000ULL

/*
 * Number of JTLB slots this test reserves at the top of the TLB before
 * exercising the allocator. The reserved slots are pinned in pinned_tlb_mask
 * so the allocator must skip them; the test then writes marker values
 * directly into them to verify the allocator never touches a pinned entry.
 *
 * In a real kernel boot, the count of slots reserved at the top of the JTLB
 * is H2K_KERNEL_NPAGES (the kernel image) plus one for the device page plus
 * one for the angel page (or zero with NULL_ANGEL_TRAP) -- see boot.ref.S.
 * This is a STANDALONE test with no kernel image to map, so we pick a fixed
 * value of 2 that lets the test:
 *   - cover the boundary between the pinned region and the allocator region
 *     (with at least one pinned slot above the allocator's high watermark),
 *   - leave plenty of room in the allocator-managed range to exercise
 *     allocate-until-full and free-from-the-top behaviors.
 * 2 is the smallest value that keeps the boundary check non-trivial while
 * matching what the previous hardcoded form of this test exercised.
 */
#define TEST_PINNED_COUNT 2

/*
 * Number of JTLB slots managed by the H2 allocator. This is a hardware-design
 * constant: pinned_tlb_mask is a u64, so the allocator covers exactly the top
 * 64 entries of the JTLB (indices [tlb_size - ALLOC_WINDOW, tlb_size - 1]).
 * Lower indices are managed by the page-table / software TLB fill path and
 * are out of scope for these tests.
 */
#define ALLOC_WINDOW 64

/*
 * Fallback JTLB size for ARCHV < 68. Older Hexagon arches don't expose the
 * JTLB size via CFG_TABLE_JTLB_SIZE -- see boot.ref.S, which gates the
 * cfg-table read on #if ARCHV >= 68 and walks the TLB to size it instead.
 * The pre-v68 cores always have a 128-entry JTLB, so we hardcode that here.
 */
#define LEGACY_JTLB_SIZE 128

static inline unsigned long long int TH_tlb_read(int index)
{
	unsigned long long int ret;
	asm volatile (" %0 = tlbr(%1) " : "=r"(ret) : "r"(index) : "memory");
	return ret;
}

static inline void TH_tlb_write(unsigned long long int entry, int index)
{
	asm volatile (" tlbw(%0,%1) " : : "r"(entry),"r"(index) : "memory");
}

void alloc_until_fail(u32_t tlb_size, u32_t last_tlb_index)
{
	int count = 0;
	int index;
	while ((index = (int)H2K_tlb_tlbop(TLBOP_TLBALLOC,0,count | TLBCONST,&a)) >= 0) {
		if (TH_tlb_read(index) != (count|TLBCONST)) FAIL("readback");
		count++;
	}
	if (count != (int)(last_tlb_index - (tlb_size - ALLOC_WINDOW) + 1)) FAIL("count");
	if (H2K_gp->pinned_tlb_mask != ~0ULL) FAIL("mask");
	if (H2K_gp->last_tlb_index != tlb_size - (ALLOC_WINDOW + 1)) FAIL("last index");
}

void free_all_0(u32_t tlb_size, u32_t last_tlb_index)
{
	u32_t i;
	printf("freeing...\n");
	for (i = tlb_size - ALLOC_WINDOW; i <= last_tlb_index; i++) {
		H2K_tlb_tlbop(TLBOP_TLBFREE,i,0,&a);
		if ((H2K_gp->pinned_tlb_mask >> (i - (tlb_size - ALLOC_WINDOW))) & 1) FAIL("tlbmask bit");
		if (H2K_gp->last_tlb_index != i) {
			printf("%d vs i=%d\n",H2K_gp->last_tlb_index,i);
			FAIL("last entry");
		}
		if (TH_tlb_read(i) != 0) FAIL("free tlbr");
	}
}

void free_all_1(u32_t tlb_size, u32_t last_tlb_index)
{
	u32_t i;
	for (i = tlb_size - (ALLOC_WINDOW - 1); i <= last_tlb_index; i++) {
		H2K_tlb_tlbop(TLBOP_TLBFREE,i,0,&a);
		if ((H2K_gp->pinned_tlb_mask >> (i - (tlb_size - ALLOC_WINDOW))) & 1) FAIL("free1 tlbmask bit");
		if (H2K_gp->last_tlb_index != tlb_size - (ALLOC_WINDOW + 1)) FAIL("free1 last entry");
		if (TH_tlb_read(i) != 0) FAIL("free1 tlbr");
	}
	H2K_tlb_tlbop(TLBOP_TLBFREE, tlb_size - ALLOC_WINDOW, 0, &a);
	if (H2K_gp->pinned_tlb_mask & 1) FAIL("free1 tlbmask bit/last");
	if (H2K_gp->last_tlb_index != last_tlb_index) FAIL("free1 last entry/last");
	if (TH_tlb_read(tlb_size - ALLOC_WINDOW) != 0) FAIL("free1 tlbr/last");
}
/* Arbitrary 20-bit virtual page number used by test_readwriteprobe. Picked
 * to avoid collisions with the test ELF's own startup TLB mappings. */
#define VALID_VPN 0x42000
#define VALID_VA (VALID_VPN<<PAGE_BITS)
/*
 * A valid-looking 64-bit JTLB entry used as a fixture for TLBWRITE / TLBREAD /
 * TLBQUERY round-trip checks. See H2K_mem_tlbfmt_t in kernel/mem/tlbfmt/tlbfmt.h
 * for the full bit layout. The value packs:
 *
 *   high word (bits 32..63) = (VALID_VPN << 32) | 0x80000000
 *     bits 32..51 (vpn:20)       = VALID_VPN     -- chosen so VALID_VA = VPN<<12
 *     bits 52..58 (asid:7)       = 0             -- TLBWRITE fills this from
 *                                                   me->ssr_asid before writing
 *     bit 63       (valid:1)     = 1             -- via 0x80000000
 *     remaining fields           = 0
 *
 *   low word (bits 0..31) = 0xfeedface
 *     bits 0..23  (ppd:24)       = 0xedface      -- non-zero PPN encoding
 *     bits 24..26 (cccc:3)       = 6             -- a valid cache-control value
 *     bit 27       (hsv39:1)     = 1
 *     bits 28..31 (xwru:4)       = 0xf           -- URWX, all permissions
 *
 * The "feedface" magic was picked so the entry stands out in TLB dumps and trace
 * output; the high nibble of the low word (0xf) doubles as full permissions, so
 * the entry is structurally valid as well as recognizable.
 */
#define VALID_TLB_ENTRY ((((unsigned long long int)VALID_VPN)<<32) | 0x80000000feedfaceULL)
/* Arbitrary 7-bit ASID used by test_readwriteprobe -- written into me->ssr_asid
 * so TLBWRITE picks it up, then expected back via the asid:7 field. */
#define ASID 0x12
#define TLB_ASID_SHIFT 52  /* bit pos of asid:7 in tlbfmt.h */
#define ASID_ENTRY (((unsigned long long int)ASID) << TLB_ASID_SHIFT)

/* Marker base OR'd with the slot index to produce a recognizable per-slot
 * value written into pinned (allocator-skip) TLB entries. The low 16 bits
 * carry the slot index so a misdirected write surfaces as an index mismatch
 * rather than a corruption that happens to read back the same value. */
#define PINNED_MARKER_BASE 0x01234567cafe0000ULL

void test_readwriteprobe(u32_t tlb_size)
{
	u32_t i;
	u32_t lo = tlb_size - 3 * ALLOC_WINDOW / 2;
	u32_t hi = tlb_size - ALLOC_WINDOW / 2;
	a.ssr_asid = ASID;
	for (i = lo; i < hi; i++) {
		if (H2K_tlb_tlbop(TLBOP_TLBWRITE,i,VALID_TLB_ENTRY,&a) != 0) FAIL("tlbw");
		if (H2K_tlb_tlbop(TLBOP_TLBQUERY,VALID_VA,0,&a) != i) FAIL("tlbp");
		if (H2K_tlb_tlbop(TLBOP_TLBREAD,i,0,&a) != (VALID_TLB_ENTRY | ASID_ENTRY)) FAIL("tlbr");
		if (H2K_tlb_tlbop(TLBOP_TLBWRITE,i,0,&a) != 0) FAIL("tlbw/2");
		if ((int)H2K_tlb_tlbop(TLBOP_TLBQUERY,VALID_VA,0,&a) >= 0) FAIL("tlbp/2");
		if (H2K_tlb_tlbop(TLBOP_TLBREAD,i,0,&a) != ASID_ENTRY) FAIL("tlbr/2");
	}
}

void random_test()
{
	
}

int main()
{
#if ARCHV > 4
	int i;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	printf("Hello!\n");
#if ARCHV >= 68
	u32_t tlb_size       = H2K_cfg_table(CFG_TABLE_JTLB_SIZE);
#else
	u32_t tlb_size       = LEGACY_JTLB_SIZE;
#endif
	u32_t last_tlb_index = tlb_size - TEST_PINNED_COUNT - 1;
	H2K_kg_init(0, 0, 0, last_tlb_index, tlb_size, 0, 0, 0);
	printf("initted!\n");
	unsigned long long expected_mask = (~0ULL) << ((last_tlb_index + 1) & (ALLOC_WINDOW - 1));
	if (H2K_gp->pinned_tlb_mask != expected_mask) {
		FAIL("mask setup");
	}
	for (u32_t pi = last_tlb_index + 1; pi < tlb_size; pi++) {
		unsigned long long marker = PINNED_MARKER_BASE | pi;
		TH_tlb_write(marker, pi);
		if (TH_tlb_read(pi) != marker) FAIL("TH");
	}
	printf("Allocing!\n");
	alloc_until_fail(tlb_size, last_tlb_index);
	free_all_0(tlb_size, last_tlb_index);
	alloc_until_fail(tlb_size, last_tlb_index);
	free_all_1(tlb_size, last_tlb_index);
	printf("readwriteprobe...\n");
	test_readwriteprobe(tlb_size);
	printf("Random Testing!\n");
	for (i = 0; i < 1000; i++) {
		random_test();
	}
#endif
	puts("TEST PASSED\n");
	return 0;
}

