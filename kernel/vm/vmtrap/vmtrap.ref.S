/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>

FUNC_START H2K_handle_trap1 .text.vm.trap .p2align 2
	crswap(r0,sgp)
	memd(r0+#CONTEXT_r3130) = r31:30
	call H2K_context_save
	/* R7 = me, r8 = ssr, r9 = htid */
	p0 = tstbit(R8,#SSR_GUEST_BIT)
	if (!p0) jump .Lvmtrap_bad_noguest
	r10.h = #hi(H2K_vmtraptab)
	r10.l = #lo(H2K_vmtraptab)
	r11 = and(r8,#0x1f)
	r10 = addasl(r10,r11,#2)
	memw(r7+#CONTEXT_continuation) = r31
	r0 = r7
	jumpr r10				/* Will return to restore_return */

FUNC_END H2K_handle_trap1

FUNC_START H2K_vmtraptab .text.vm.trap .p2align 5
	jump H2K_vmtrap_version		// 0
	jump H2K_vmtrap_return		// 1
	jump H2K_vmtrap_setvec		// 2
	jump H2K_vmtrap_setie		// 3
	jump H2K_vmtrap_getie		// 4
	jump H2K_vmtrap_intop		// 5
	jump H2K_vmtrap_bad		// 6
	jump H2K_vmtrap_bad		// 7
	jump H2K_vmtrap_bad		// 8
	jump H2K_vmtrap_bad		// 9
	jump H2K_vmtrap_clrmap		// a
	jump H2K_vmtrap_newmap		// b
	jump H2K_vmtrap_bad		// c
	jump H2K_vmtrap_cachectl	// d
	jump H2K_vmtrap_get_pcycles	// e
	jump H2K_vmtrap_set_pcycles	// f
	jump H2K_vmtrap_wait		// 10
	jump H2K_vmtrap_yield		// 11
	jump H2K_vmtrap_start		// 12
	jump H2K_vmtrap_stop		// 13
	jump H2K_vmtrap_vmpid		// 14
	jump H2K_vmtrap_setregs		// 15
	jump H2K_vmtrap_getregs		// 16
	jump H2K_vmtrap_bad		// 17
	jump H2K_vmtrap_timer		// 18
	jump H2K_vmtrap_pmuctrl		// 19
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
FUNC_START H2K_vmtrap_bad .text.vm.trap .p2align 2
.Lvmtrap_bad_noguest:
	r3 = r7			/* me */
	r0 = #0			/* BADVA */
	r1 = #0x1B		/* No Guest Permission */
	r2 = #ERROR_GEVB_OFFSET	/* Vec Offset */
	jump H2K_vm_event

FUNC_END H2K_vmtrap_bad

