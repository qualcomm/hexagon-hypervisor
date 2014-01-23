/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>

#define SET_REG(REGNO,VAL) \
	r0.h = #hi(VAL); \
	r0.l = #lo(VAL); \
	REGNO = r0;

#if ARCHV <= 3
#define GET_HTID(REG) \
	REG = ssr; \
	REG = extractu(REG,#3,#19);

#else
#define GET_HTID(REG) \
	REG = HTID;

#endif

#define TEST_ONE_PAIR(REGHI,REGLO,PAIRNAME,UNIQ) \
	SET_REG(r0,UNIQ); \
	allocframe(#8); \
	memw(r30+#-8) = r0; \
	SET_REG(r0,preprintstr); \
	/*call printf; */ \
	deallocframe; \
	SET_REG(REGHI,0xDEA00000+UNIQ); \
	SET_REG(REGLO,0xBEEF0000+UNIQ+1); \
	crswap(r0,sgp0); \
	call H2K_context_save; \
	r0 = sgp0; \
	r3:2 = memd(r0+#CONTEXT_##PAIRNAME); \
	SET_REG(r5,0xDEA00000+UNIQ); \
	SET_REG(r4,0xBEEF0000+UNIQ+1); \
	p0 = cmp.eq(r3,r5); \
	p1 = cmp.eq(r2,r4); \
	p0 = and(p0,p1); \
	SET_REG(r5,H2K_kg); \
	p2 = cmp.eq(r28,r5); \
	p3 = cmp.eq(r29,#0); \
	p0 = and(p0,p1); \
	p0 = and(p0,!p3); \
	if (p0) jump 1f; \
	jump testFAIL; \
1: \
	/* Also test me/ssr/htid */ \
	r2 = sgp0; \
	r3 = r7; \
	p0 = cmp.eq(r2,r3); \
	r2 = ssr; \
	r3 = r8; \
	p1 = cmp.eq(r2,r3); \
	GET_HTID(r2); \
	r3 = r9; \
	p2 = cmp.eq(r2,r3); \
	p0 = and(p0,p1); \
	p0 = and(p0,p2); \
	if (p0) jump 1f; \
	jump testFAIL; \
1: \
	SET_REG(r2,1f); \
	elr = r2; \
	SET_REG(r3,0x00000000); \
	SET_REG(r2,0x000a0000); \
	r0 = sgp0; \
	memd(r0+#CONTEXT_ccrssr) = r3:2; \
	SET_REG(REGHI,0); \
	SET_REG(REGLO,0); \
	jumpr r31; \
	jump testFAIL; \
1: \
	r5 = REGHI; \
	r4 = REGLO; \
	SET_REG(r3,0xDEA00000+UNIQ); \
	SET_REG(r2,0xBEEF0000+UNIQ+1); \
	p0 = cmp.eq(r3:2,r5:4); \
	if (p0) jump 1f; \
	jump testFAIL; \
1: \
	r29 = add(r30,#-8); \
	allocframe(#8); \
	SET_REG(r0,UNIQ); \
	memw(r30+#-8) = r0; \
	SET_REG(r0,printstr); \
	r2.h = #hi(_SDA_BASE_); \
	r2.l = #lo(_SDA_BASE_); \
	gp = r2; \
	call printf; \
	deallocframe

	.text
	.global dotest_asm
dotest_asm:
	/* SAVE r31:30 */
	allocframe(#8)
	allocframe(#8)
	memd(r0+#CONTEXT_r3130) = r31:30
	sgp0 = r0
	r1.h = #hi(SAVE)
	r1.l = #lo(SAVE)
	memd(r1) = r31:30
TEST_ONE_PAIR(r1,r0,r0100,0)
TEST_ONE_PAIR(r3,r2,r0302,2)
TEST_ONE_PAIR(r5,r4,r0504,4)
TEST_ONE_PAIR(r7,r6,r0706,6)
TEST_ONE_PAIR(r9,r8,r0908,8)
TEST_ONE_PAIR(r11,r10,r1110,10)
TEST_ONE_PAIR(r13,r12,r1312,12)
TEST_ONE_PAIR(r15,r14,r1514,14)
TEST_ONE_PAIR(r17,r16,r1716,16)
TEST_ONE_PAIR(r19,r18,r1918,18)
//TEST_ONE_PAIR(r21,r20,r2120,20)
//TEST_ONE_PAIR(r23,r22,r2322,22)
//TEST_ONE_PAIR(r25,r24,r2524,24)
//TEST_ONE_PAIR(r27,r26,r2726,26)
TEST_ONE_PAIR(r29,r28,r2928,28)
TEST_ONE_PAIR(lc0,sa0,lc0sa0,100)
//TEST_ONE_PAIR(lc1,sa1,lc1sa1,102)
//TEST_ONE_PAIR(m1,m0,m1m0,104)
TEST_ONE_PAIR(usr,p3:0,usrp30,(-0xDEA00000))
//TEST_ONE_PAIR(gp,ugp,gpugp,0x01000000)
#if ARCHV >= 4
//TEST_ONE_PAIR(cs1,cs0,cs1cs0,108)
//TEST_ONE_PAIR(g1,g0,gssr_gelr,112)
//TEST_ONE_PAIR(g3,g2,gbadva_gosp,114)
#endif
	/* RESTORE r31:30 */
	r0.h = #hi(SAVE)
	r0.l = #lo(SAVE)
	r3:2 = memd(r0)
	p0 = cmp.eq(r3:2,r31:30)
	if (!p0) jump testFAIL
	deallocframe
	deallocframe
	jumpr r31

testFAIL:
	deallocframe
	r0.h = #hi(_SDA_BASE_)
	r0.l = #lo(_SDA_BASE_)
	gp = r0
	r0.h = #hi(str)
	r0.l = #lo(str)
	jump FAIL

	.data
	.p2align 4
SAVE:
	.word 0
	.word 0
str:
	.asciz "Failure on save or restore"
printstr:
	.asciz "OK: %d\n"
preprintstr:
	.asciz "Trying: %d\n"

