/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>

FUNC_START H2K_handle_trap1 .text.event.trap1 .p2align 2
	crswap(r0,sgp)
	memd(r0+#CONTEXT_r3130) = r31:30
	memd(r0+#CONTEXT_r2928) = r29:28
	r30 = p3:0
	r31 = usr
	memd(r0+#CONTEXT_sr_preds) = r31:30

	memd(r0+#CONTEXT_r2726) = r27:26
	memd(r0+#CONTEXT_r2524) = r25:24
	memd(r0+#CONTEXT_r2322) = r23:22
	memd(r0+#CONTEXT_r2120) = r21:20
	memd(r0+#CONTEXT_r1918) = r19:18
	memd(r0+#CONTEXT_r1716) = r17:16
	memd(r0+#CONTEXT_r1514) = r15:14
	memd(r0+#CONTEXT_r1312) = r13:12
	memd(r0+#CONTEXT_r1110) = r11:10
	memd(r0+#CONTEXT_r0908) = r9:8
	memd(r0+#CONTEXT_r0706) = r7:6
	memd(r0+#CONTEXT_r0504) = r5:4
	memd(r0+#CONTEXT_r0302) = r3:2

	r13 = ugp
	r12 = gp
	r9 = lc0
	r8 = sa0
	r7 = lc1
	r6 = sa1
	r5 = m1
	r4 = m0
	r3 = ssr
	r2 = elr
#if __QDSP6_ARCH__ >= 4
	r11 = ccr
	r17 = cs1
	r16 = cs0
#endif

	memw(r0+#CONTEXT_ccr) = r11
	memd(r0+#CONTEXT_ugpgp) = r13:12
	memd(r0+#CONTEXT_sr_preds) = r31:30
	memd(r0+#CONTEXT_cs1cs0) = r17:16
	memd(r0+#CONTEXT_lc0sa0) = r9:8
	memd(r0+#CONTEXT_lc1sa1) = r7:6
	memd(r0+#CONTEXT_m1m0) = r5:4
	memd(r0+#CONTEXT_ssrelr) = r3:2

	/* Save off last registers */
	r21 = r0
	crswap(r0,sgp)
	memd(r21+#CONTEXT_r0100) = r1:0
	r0 = r21

#if __QDSP6_ARCH__ <= 3
	r29.h = #hi(H2K_stacks)
	r29.l = #lo(H2K_stacks)
	r28 = ssr
	r28 = extractu(r28,#3,#19)
	r2 = #KERNEL_STACK_SIZE
	r29 += mpyi(r28,r2)
#else
	r28 = HTID
	r29 = #KERNEL_STACK_SIZE
	r29 = add(##H2K_stacks,mpyi(r28,r29))
#endif

	r16.h = #hi(H2K_kg)
	r16.l = #lo(H2K_kg)

#if __QDSP6_ARCH__ >= 4
	r3:2 = g1:0
	memd(r0+#CONTEXT_gelr_gbadva) = r3:2
	r3:2 = g3:2
	memd(r0+#CONTEXT_gssr_gosp) = r3:2
#endif

	/* If not in guest mode, permission error */
	r3 = ssr
	r4 = p3:0
	p0 = tstbit(r3,#SSR_GUEST_BIT)
	{
		p3:0 = r4
		if (!p0) jump .Lvmtrap_bad_noguest
	}
	r1.h = #hi(H2K_vmtraptab)
	r1.l = #lo(H2K_vmtraptab)
	r3 = and(r3,#0x1f)
	r1 = addasl(r1,r3,#2)

	/* EJP: FIXME: how to set continuation for things like yield? */
	callr r1

.Lreturn:
	/* Restore all state and return */
	r0 = sgp

#if __QDSP6_ARCH__ >= 4
	r5:4 = memd(r0+#CONTEXT_gelr_gbadva)
	g1:0 = r5:4
	r5:4 = memd(r0+#CONTEXT_gssr_gosp)
	g3:2 = r5:4
#endif

	r3:2 = memd(r0+#CONTEXT_ssrelr)
	r5:4 = memd(r0+#CONTEXT_m1m0)
	r7:6 = memd(r0+#CONTEXT_lc1sa1)
	r9:8 = memd(r0+#CONTEXT_lc0sa0)
	r11:10 = memd(r0+#CONTEXT_sr_preds)
	r13:12 = memd(r0+#CONTEXT_ugpgp)
	r17:16 = memd(r0+#CONTEXT_cs1cs0)
	r14 = memw(r0+#CONTEXT_ccr)
	elr = r2
	ssr = r3
	m0 = r4
	m1 = r5
	sa1 = r6
	lc1 = r7
	sa0 = r8
	lc0 = r9
	p3:0 = r10
	usr = r11
	gp = r12
	ugp = r13
#if __QDSP6_ARCH__ >= 4
	ccr = r14
	cs1 = r17
	cs0 = r16
#endif
	r3:2 = memd(r0+#CONTEXT_r0302)
	r5:4 = memd(r0+#CONTEXT_r0504)
	r7:6 = memd(r0+#CONTEXT_r0706)
	r9:8 = memd(r0+#CONTEXT_r0908)
	r11:10 = memd(r0+#CONTEXT_r1110)
	r13:12 = memd(r0+#CONTEXT_r1312)
	r15:14 = memd(r0+#CONTEXT_r1514)
	r17:16 = memd(r0+#CONTEXT_r1716)
	r19:18 = memd(r0+#CONTEXT_r1918)
	r21:20 = memd(r0+#CONTEXT_r2120)
	r23:22 = memd(r0+#CONTEXT_r2322)
	r25:24 = memd(r0+#CONTEXT_r2524)
	r27:26 = memd(r0+#CONTEXT_r2726)
	r29:28 = memd(r0+#CONTEXT_r2928)
	r31:30 = memd(r0+#CONTEXT_r3130)
	r1:0 = memd(r0+#CONTEXT_r0100)

	rte
FUNC_END H2K_handle_trap1

FUNC_START H2K_vmtraptab .text.event.trap1 .p2align 5
	jump H2K_vmtrap_bad		// 0
	jump H2K_vmtrap_return		// 1
	jump H2K_vmtrap_setvec		// 2
	jump H2K_vmtrap_setie		// 3
	jump H2K_vmtrap_getie		// 4
	jump H2K_vmtrap_swi		// 5
	jump H2K_vmtrap_iack		// 6
	jump H2K_vmtrap_setimask	// 7
	jump H2K_vmtrap_getimask	// 8
	jump H2K_vmtrap_iconf		// 9
	jump H2K_vmtrap_clrmap		// a
	jump H2K_vmtrap_register_ptb	// b
	jump H2K_vmtrap_bad		// c
	jump H2K_vmtrap_cachectl	// d
	jump H2K_vmtrap_get_pcycles	// e
	jump H2K_vmtrap_set_pcycles	// f
	jump H2K_vmtrap_wait		// 10
	jump H2K_vmtrap_yield		// 11
	jump H2K_vmtrap_start		// 12
	jump H2K_vmtrap_stop		// 13
	jump H2K_vmtrap_vmpid		// 14
	jump H2K_vmtrap_bad		// 15
	jump H2K_vmtrap_bad		// 16
	jump H2K_vmtrap_bad		// 17
	jump H2K_vmtrap_bad		// 18
	jump H2K_vmtrap_bad		// 19
	jump H2K_vmtrap_bad		// 1a
	jump H2K_vmtrap_bad		// 1b
	jump H2K_vmtrap_bad		// 1c
	jump H2K_vmtrap_bad		// 1d
	jump H2K_vmtrap_bad		// 1e
	jump H2K_vmtrap_bad		// 1f
FUNC_END H2K_vmtraptab

/* Invalid VM Trap because:
 * * Was not called from guest mode, or 
 * * Was not a valid trap number
 * Stack/kgp set up
 * Most Special regs not saved (LC/SA 0/1, M01, CS01, etc)
 * 
 * So...
 *  Save extra special regs (not needed in opt)
 *  Call H2K_vm_event
 *  Restore extra special regs (not needed in opt)
 */
FUNC_START H2K_vmtrap_bad .text.event.trap1 .p2align 2
	allocframe(#0)
	r3 = r0			/* me */
	r0 = #0			/* BADVA */
	r1 = #0x1B		/* No Guest Permission */
	r2 = #ERROR_GEVB_OFFSET	/* Vec Offset */
	call H2K_vm_event
	deallocframe
	jumpr r31
.Lvmtrap_bad_noguest:
	call H2K_vmtrap_bad
	jump .Lreturn

FUNC_END H2K_vmtrap_bad

