/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>
#include <max.h>

#define STLB_MAX_PAGES_FOR_VA_INV 4

/*
H2K_mem_stlb_find(va,asid,myinfo)
IDX = (va >> STLB_SHIFT) & (STLB_MAX_SETS-1) (and maybe ^ ASID?)
IDX:SHIFT = bitsplit(idx,5)
ASIDVA = (va >> 12) | (asid << 20)
BITS = memw(myinfo->valids+IDX<<#2)
p = TSTBIT(BITS,SHIFT)
if (!p) return NOT_FOUND
loop0(.Loop,STLB_MAX_WAYS)
PTR = myinfo->baseaddr+mpyi(IDX,STLB_MAX_WAYS)
.Loop:
	VAL = memd(PTR++#8)
	p = tlbmatch(VAL,ASIDVA)
	if (p) jump:nt found // or set result or whatever
	endloop
notfound:
	return NOT_FOUND
found:
	PTR and VAL are useful
*/

#if STLB_INFO_valids != 0
#fatal assumed valids at beginning of struct
#endif

/* H2K_mem_stlb_lookup(u32_t va, u32_t asid, H2K_thread_context *me) */
FUNC_START H2K_mem_stlb_lookup .text.mem.stlb .p2align 5
#define VA r0
#define ASID_IN r1
#define ASID_VA_IN r1:0
#define ME r2
#define BASEADDR r3
#define ASIDVA r4
#define JUNK r5
#define JUNK_ASIDVA r5:4
#define BITS r6
#define IDX r7
#define OFF r8
#define STLB r9
#define SHIFT r10
#define PTR r11
#define SET r12
#define ISIZE r13
#define VAL r15:14
	{
		STLB = memw(H2K_GP+#KG_stlbptr)
		if (cmp.eq(STLB.new,#0)) jump:nt .Lno_stlb
		IDX = extractu(VA,#STLB_MAX_SETS_LOG2-5,#STLB_SHIFT+5)
		SHIFT = extractu(VA,#5,#STLB_SHIFT)
	}
	{
		SET = extractu(VA, #STLB_MAX_SETS_LOG2, #STLB_SHIFT)
		ISIZE = #STLB_INFO_SIZE
	}
	{
		JUNK_ASIDVA = lsr(ASID_VA_IN,#12)
		STLB += mpyi(ASID_IN, ISIZE)
#undef VA
#undef ASID_IN
#undef ASID_VA_IN
#define RET r1:0
		RET = #0
	}
	{
		BASEADDR = memw(STLB+#STLB_INFO_baseaddr)
		BITS = memw(STLB+IDX<<#2)
		OFF = mpyi(SET, #STLB_MAX_WAYS*8)
		loop0(1f,#STLB_MAX_WAYS-2)
	}
	{
		p0 = tstbit(BITS,SHIFT)
		if (!p0.new) jump:nt .Lnotfound
		PTR = add(BASEADDR,OFF)	// close to the value I want next, this is the +0 address I'm loading...
		VAL = memd(BASEADDR+OFF<<#0)
	}
#undef STLB
#undef OFF
#define LASTVAL r9:8
	{
		p0 = tlbmatch(VAL,ASIDVA)
		VAL = memd(PTR+#8)	// ptr was idx 0, we want idx 1
		PTR = add(PTR,#16)	// and point ptr to idx 2.
		LASTVAL = VAL
	}
	.falign
1:
	{
		p0 = tlbmatch(VAL,ASIDVA)
		if (p0) RET = LASTVAL
		VAL = memd(PTR++#8)
		LASTVAL = VAL
	}:endloop0
	{
		p0 = tlbmatch(VAL,ASIDVA)
		if (p0) RET = LASTVAL
	}
.Lnotfound:
	{
		if (p0) RET = VAL
		jumpr r31
	}
#undef ME
#undef BASEADDR
#undef ASIDVA
#undef JUNK
#undef JUNK_ASIDVA
#undef BITS
#undef IDX
#undef LASTVAL
#undef SHIFT
#undef PTR
#undef VAL
#undef RET
#undef SET
#undef ISIZE

FUNC_END H2K_mem_stlb_lookup

/* H2K_mem_stlb_add(u32_t va, u32_t asid, H2K_mem_tlbfmt_t entry, H2K_thread_context *me) */
FUNC_START H2K_mem_stlb_add .text.mem.stlb .p2align 5
#define VA r0
#define ASID_IN r1
#define ENTRY r3:2
// #define ME r4 -- unused
#define ISIZE r4
#define SET r5
#define MYINFO r6
#define RNDIDX r7
#define BIT r8
#define VALID_VAL r9
#define ROWADDR r10
#define ROWOFF r11
#define PTR r12
#define IDX r13
#define DATA r15:14
#define DATAHI r15
#define DATALO r14
	{
		IDX = extractu(VA,#STLB_MAX_SETS_LOG2-5,#STLB_SHIFT+5)
		MYINFO = memw(H2K_GP+#KG_stlbptr)
		RNDIDX = pcyclelo
		if (cmp.eq(MYINFO.new,#0)) jump:nt .Lno_stlb
	}
	{
		SET = extractu(VA, #STLB_MAX_SETS_LOG2, #STLB_SHIFT)
		ISIZE = #STLB_INFO_SIZE
	}
	{
		BIT = extractu(VA,#5,#STLB_SHIFT)
		MYINFO += mpyi(ASID_IN, ISIZE)
	}
#undef VA
#define TESTASID r0
	{
		VALID_VAL = memw(MYINFO+IDX<<#2)
		loop0(1f,#STLB_MAX_WAYS-2)
		ROWADDR = memw(MYINFO+#STLB_INFO_baseaddr)
		ROWOFF = mpyi(SET, #STLB_MAX_WAYS*8)
	}
#undef SET
#undef ISIZE
#define ZERO r5:4
	{
		p0 = tstbit(VALID_VAL,BIT)
		if (p0.new) jump:t .Lstlb_add_insert
		RNDIDX = and(RNDIDX,#STLB_MAX_WAYS-1)
		ROWADDR = add(ROWADDR,ROWOFF)
	}
	{
		DATA = memd(ROWADDR+#STLB_MAX_WAYS*8-8)
		PTR = add(ROWADDR,#STLB_MAX_WAYS*8-16)
		VALID_VAL = setbit(VALID_VAL,BIT)
	}
	{
		DATA = memd(PTR++#-8)
		TESTASID = extractu(DATAHI,#7,#20)
		ZERO = #0
	}
	.falign
1:
	{
		DATA = memd(PTR++#-8)
		TESTASID = extractu(DATAHI,#7,#20)
		p0 = cmp.eq(TESTASID,ASID_IN)
		if (p0.new) memd(PTR+#16) = ZERO
	}:endloop0
	{
		TESTASID = extractu(DATAHI,#7,#20)
		p0 = cmp.eq(TESTASID,ASID_IN)
		if (p0.new) memd(PTR+#16) = ZERO
	}
	{
		p0 = cmp.eq(TESTASID,ASID_IN)
		if (p0.new) memd(PTR+#8) = ZERO
	}
.Lstlb_add_insert:
	{
		memw(MYINFO+IDX<<#2) = VALID_VAL	// only valid after we've cleaned
		memd(ROWADDR+RNDIDX<<#3) = ENTRY
		jumpr r31
	}
.Lno_stlb:
	{
		r1:0 = #0
		jumpr r31
	}
#undef TESTASID
#undef VA
#undef ASID_IN
#undef ENTRY
#undef IDX
#undef RNDIDX
#undef ZERO
#undef ROWADDR
#undef ROWOFF
#undef PTR
#undef MYINFO
#undef DATA
#undef DATAHI
#undef DATALO
FUNC_END H2K_mem_stlb_add

/* H2K_mem_stlb_invalidate_va(u32_t va, u32_t count, u32_t asid, H2K_thread_context *me) */
FUNC_START H2K_mem_stlb_invalidate_va .text.mem.stlb .p2align 4
#define VA r0
#define COUNT r1
#define COUNT_VA r1:0
#define ASID r2
#define ME r3
#define STLB r4
#define BITS r5
#define BASEADDR r6
#define IDX r7
#define SHIFT r8
#define PTR r9
#define NEXTVPN r10
#define ISIZE r12
#define SET R13
#define DZERO r15:14

	{
		STLB = memw(H2K_GP+#KG_stlbptr)
		COUNT_VA = vlsrw(COUNT_VA,#PAGE_BITS)
		IDX = extractu(VA,#STLB_MAX_SETS_LOG2-5,#STLB_SHIFT+5)
		if (cmp.eq(STLB.new,#0)) jump:nt .Lno_stlb
	}
#undef VA
#undef COUNT_VA
#define VPN r0
#define COUNT_VPN r1:0
	{
		SET = extractu(VPN, #STLB_MAX_SETS_LOG2, #STLB_SHIFT - PAGE_BITS)
		ISIZE = #STLB_INFO_SIZE
		VPN |= asl(ASID,#20)
		p0 = cmp.gt(COUNT,#STLB_MAX_PAGES_FOR_VA_INV-1)
	{
		if (p0) jump H2K_mem_stlb_invalidate_asid
		if (p0) r1:0 = combine(ME,ASID)
		STLB += mpyi(ASID, ISIZE)
	}
#undef ISIZE
#undef ME
#define LSET r3
	{
		LSET = SET
		BASEADDR = memw(STLB+#STLB_INFO_baseaddr)
		loop1(.Linvalidate_va_loop,COUNT)
	}
#undef SET
#define VAL r13:12

	.falign
.Linvalidate_va_loop:
	{
		BITS = memw(STLB+IDX<<#2)
		PTR = add(BASEADDR, mpyi(#(STLB_MAX_WAYS*8), LSET))
		DZERO = #0
		SHIFT = extractu(VPN,#5,#STLB_SHIFT-PAGE_BITS)
	}
	{
		p0 = tstbit(BITS,SHIFT)
		if (!p0.new) jump:nt .Linvalidate_va_loop_notfound
		VAL = memd(PTR+#STLB_MAX_WAYS*8-8)
		NEXTVPN = add(VPN,#1)
	}
	{
		loop0(.Linvalidate_va_find_loop,#STLB_MAX_WAYS)
		VAL = memd(PTR+#STLB_MAX_WAYS*8-16)
		p0 = tlbmatch(VAL,VPN)
		PTR = add(PTR,#STLB_MAX_WAYS*8-24)
	}
	.falign
.Linvalidate_va_find_loop:
	{
		VAL = memd(PTR++#-8)
		p0 = tlbmatch(VAL,VPN)
		if (p0) memd(PTR+#16) = DZERO
	}:endloop0
.Linvalidate_va_loop_notfound:
	{
		VPN = NEXTVPN
		IDX = extractu(NEXTVPN,#STLB_MAX_SETS_LOG2-5,#STLB_SHIFT-PAGE_BITS+5)
		LSET = extractu(NEXTVPN, #STLB_MAX_SETS_LOG2, #STLB_SHIFT - PAGE_BITS)
	}:endloop1
	jumpr r31
#undef VA
#undef ASID_IN
#undef ENTRY
#undef ME
#undef TMP
#undef RNDIDX
#undef BIT
#undef VALID_VAL
#undef ROWADDR
#undef ROWOFF
#undef PTR
#undef STLB
#undef DATA
#undef DATAHI
#undef DATALO
FUNC_END H2K_mem_stlb_invalidate_va

FUNC_START H2K_mem_stlb_invalidate_asid .text.mem.stlb .p2align 4
#define ASID_IN r0
#define PTR r1
#define ISIZE r2
	{
		PTR = memw(H2K_GP+#KG_stlbptr)
		loop0(1f,#STLB_MAX_SETS/(32*8))
		ISIZE = #STLB_INFO_SIZE
	}
	{
		p0 = cmp.eq(PTR,#0)
		if (p0.new) jump:nt 9f
		PTR += mpyi(ASID_IN, ISIZE)
	}
	.falign
1:
	{
		dczeroa(PTR)
		PTR = add(PTR,#32)
	}:endloop0
9:
	jumpr r31
#undef ASID_IN
#undef PTR
#undef ISIZE
FUNC_END H2K_mem_stlb_invalidate_asid
