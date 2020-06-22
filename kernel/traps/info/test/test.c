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
#include <cfg_table.h>
#include <l2cache.h>
#include <thread.h>
#include <max.h>
#define UNIT_BYTES (ALLOC_UNIT * sizeof(H2K_mem_alloc_tag_t))

unsigned int alloc_heap_size = H2K_ALLOC_HEAP_SIZE;
H2K_mem_alloc_tag_t Heap[H2K_ALLOC_HEAP_SIZE] __attribute__((aligned(ALLOC_UNIT))) = {{{.size = 0, .free = 0}}};
#undef H2K_ALLOC_HEAP_SIZE
#include <symbols.h>

static H2K_thread_context a;
static struct H2K_vmblock_struct vmblock;

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
	u32_t phys_offset=0, devpage_offset=0, last_tlb_index=125, tlb_size=128;
	H2K_kg_init(phys_offset, devpage_offset, last_tlb_index, tlb_size);
	H2K_l2cache_init();
	H2K_mem_alloc_init(Heap, alloc_heap_size);
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

	a.next = a.prev = &a; // single-thread usage
	a.prio = 0;
	a.hthread = 0;
	a.tid = 0;
	a.id.raw = 0; // thread vmidx, cpuidx & id_vint all clear
	a.vmblock = &vmblock; // thread vmblock ptr set to struct

	asm volatile ( "%0 = s35\n" : "=r" (val));
	if (H2K_trap_info(INFO_LIVELOCK, &a) != val) FAIL("LIVELOCK");

	val = H2K_cfg_table(CFG_TABLE_SSBASE) << CFG_TABLE_SHIFT;
	if (H2K_trap_info(INFO_SSBASE, &a) != val) FAIL("SSBASE");

	val = H2K_kg.last_tlb_index+1;
	if (H2K_trap_info(INFO_TLB_FREE, &a) != val) FAIL("TLB_FREE");

	val = H2K_kg.tlb_size;
	if (H2K_trap_info(INFO_TLB_SIZE, &a) != val) FAIL("TLB_SIZE");

	val = (H2K_LINK_ADDR - H2K_kg.phys_offset);
	if (H2K_trap_info(INFO_PHYSADDR, &a) != val) FAIL("PHYSADDR");

	val = (H2K_cfg_table(CFG_TABLE_L2TCM) << CFG_TABLE_SHIFT);
	if (H2K_trap_info(INFO_TCM_BASE, &a) != val) FAIL("TCM_BASE");

	val = H2K_kg.l2size;
	if (H2K_trap_info(INFO_L2MEM_SIZE, &a) != val) FAIL("L2MEM_SIZE");

	val = H2K_kg.tcm_size;
	if (H2K_trap_info(INFO_TCM_SIZE, &a) != val) FAIL("TCM_SIZE");

	val = H2K_PAGESIZE;
	if (H2K_trap_info(INFO_H2K_PGSIZE, &a) != val) FAIL("H2K_PAGESIZE");

	val = (u32_t)&H2K_KERNEL_NPAGES;
	if (H2K_trap_info(INFO_H2K_NPAGES, &a) != val) FAIL("H2K_NPAGES");

	val = ((H2K_cfg_table(CFG_TABLE_SSBASE) << CFG_TABLE_SHIFT) + L2VIC_OFFSET);
	if (H2K_trap_info(INFO_L2VIC_BASE, &a) != val) FAIL("L2VIC_BASE");

	val = ((H2K_cfg_table(CFG_TABLE_SSBASE) << CFG_TABLE_SHIFT) + TIMER_OFFSET);
	if (H2K_trap_info(INFO_TIMER_BASE, &a) != val) FAIL("TIMER_BASE");

#if ARCHV >= 60
	val = TIMER_INT_CORE_V60;
#elif ARCHV >= 5
	val = TIMER_INT_CORE_V5;
#else
	val = TIMER_INT_CORE_V4;
#endif
	if (H2K_trap_info(INFO_TIMER_INT, &a) != val) FAIL("TIMER_INT");

	val = H2K_kg.kernel_error;
	if (H2K_trap_info(INFO_ERROR, &a) != val) FAIL("ERROR");

	val = H2K_kg.hthreads_mask;
	if (H2K_trap_info(INFO_HTHREADS, &a) != val) FAIL("HTHREADS");

	val = (H2K_kg.l2tags > 0 ? (1 << H2K_kg.l2tags) * L2_TAG_CHUNK : 0);
	if (H2K_trap_info(INFO_L2TAG_SIZE, &a) != val) FAIL("L2TAG_SIZE");

	val = H2K_cfg_table(CFG_TABLE_L2REGS) << CFG_TABLE_SHIFT;
	if (H2K_trap_info(INFO_L2CFG_BASE, &a) != val) FAIL("L2CFG_BASE");

	val = H2K_cfg_table(CFG_TABLE_CLADEREGS) << CFG_TABLE_SHIFT;
	if (H2K_trap_info(INFO_CLADE_BASE, &a) != val) FAIL("CLADE_BASE");

	val = H2K_get_cfgbase();
	if (H2K_trap_info(INFO_CFGBASE, &a) != val) FAIL("CFGBASE");

	val = H2K_kg.hvx_vlength;
	if (H2K_trap_info(INFO_HVX_VLENGTH, &a) != val) FAIL("HVX_VLENGTH");

	val = H2K_kg.hvx_contexts;
	if (H2K_trap_info(INFO_HVX_CONTEXTS, &a) != val) FAIL("HVX_CONTEXTS");

#ifdef DO_EXT_SWITCH
	val = a.vmblock->do_ext;
#else
	val = 0;
#endif
	if (H2K_trap_info(INFO_HVX_SWITCH, &a) != val) FAIL("HVX_SWITCH");

#if ARCHV > 65
	val = (H2K_cfg_table(CFG_TABLE_VTCM_BASE) << CFG_TABLE_SHIFT);
#elif ARCHV == 65
	val = (H2K_kg.hvx_contexts > 0 ? H2K_kg.tcm_base + EXT_HVX_VTCM_OFFSET : 0);
#else
	val = 0;
#endif
	if (H2K_trap_info(INFO_VTCM_BASE, &a) != val) FAIL("VTCM_BASE");

#if ARCHV > 65
	val = H2K_cfg_table(CFG_TABLE_VTCM_SIZE);
#elif ARCHV == 65
	val = (H2K_kg.hvx_contexts > 0 ? EXT_HVX_VTCM_SIZE : 0);
#else
	val = 0;
#endif
	if (H2K_trap_info(INFO_VTCM_SIZE, &a) != val) FAIL("VTCM_SIZE");

#if ARCHV > 65
	val = (H2K_cfg_table(CFG_TABLE_ECC_BASE) << CFG_TABLE_SHIFT);
#elif ARCHV == 65
	val = 0;
#else
	val = 0;
#endif
	if (H2K_trap_info(INFO_ECC_BASE, &a) != val) FAIL("ECC_BASE");

#if ARCHV > 65
	val = H2K_cfg_table(CFG_TABLE_L2_LINE_SZ);
#elif ARCHV == 65
	val = 64;
#else
	val = 64;
#endif
	if (H2K_trap_info(INFO_L2_LINE_SZ, &a) != val) FAIL("L2_LINE_SZ");

#if ARCHV > 65
	val = H2K_cfg_table(CFG_TABLE_AUDIO_EXT);
#elif ARCHV == 65
	val = 0;
#else
	val = 0;
#endif
	if (H2K_trap_info(INFO_AUDIO_EXT, &a) != val) FAIL("AUDIO_EXT");

	puts("TEST PASSED\n");
	return 0;
}
