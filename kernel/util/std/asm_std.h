/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_ASM_STD_H
#define H2K_ASM_STD_H 1

#ifndef ASM
#define ASM 1
#endif

#include <asm_offsets.h>

#define H2K_GP R28

#define NULL 0

#define H2K_LANG_IS_ASM 1

.macro FUNC_START name, section, align, amt
	.section \section,"ax",@progbits
	\align \amt
	.global \name
	.type \name,@function
\name:
.endm

.macro FUNC_END name
	.size \name,.-\name
.endm

.macro DATA_START name, section, align, amt
	.section \section,"aw",@progbits
	\align \amt
	.global \name
	.type \name,@object
\name:
.endm

.macro DATA_END name
	.size \name,.-\name
.endm

#if ARCHV >= 4
#define sgp sgp0
#endif

/* BKL is an in-memory ticket spinlock at KG_bkl.  Three scratch registers and a
 * predicate register are required; all four are clobbered. */

#define ASM_TICKET_LOCK(addr, tmp1, tmp2, tmp_pred) \
1: \
	tmp1 = memw_locked(addr) ; \
	{ \
		tmp2 = lsr(tmp1,#16); \
		tmp1 = add(tmp1, # ## #0x10000); \
	} \
	memw_locked(addr,tmp_pred) = tmp1 ;\
	{ \
		if (!tmp_pred) jump:nt 1b ;\
		tmp_pred = cmph.eq(tmp1,tmp2) ;\
		if (tmp_pred.new) jump:t 3f ;\
	} \
2: \
	tmp1 = memuh(addr); \
	{ tmp_pred = cmph.eq(tmp1,tmp2); if (tmp_pred.new) jump:t 3f } \
	/* pause */ \
	jump 2b; \
3:

#define ASM_TICKET_UNLOCK(addr,tmp1,tmp2,tmp_pred) \
1: \
	{ tmp2 = #1; tmp1 = memw_locked(addr); } \
	tmp1 = vaddh(tmp1,tmp2); /* add halfwords separately to avoid carry */ \
	memw_locked(addr,tmp_pred) = tmp1; \
	if (!tmp_pred) jump:nt 1b; \

#ifdef SOFT_BKL
#define ASM_BKL_LOCK(A,B,C,P) A = add(H2K_GP,#KG_bkl); ASM_TICKET_LOCK(A,B,C,P)
#define ASM_BKL_UNLOCK(A,B,C,P) A = add(H2K_GP,#KG_bkl); ASM_TICKET_UNLOCK(A,B,C,P)
#else
#define ASM_BKL_LOCK(tmp0, tmp1, tmp2, tmp_pred)   K0LOCK
#define ASM_BKL_UNLOCK(tmp0, tmp1, tmp2, tmp_pred) K0UNLOCK
#endif /* SOFT_BKL */

#endif

