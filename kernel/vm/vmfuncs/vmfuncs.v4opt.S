/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_offsets.h>
#include <asm_std.h>
#include <max.h>
#include <vmdefs.h>
#include <idtype.h>

#define ME r0

FUNC_START H2K_vmtrap_version .text.vm.funcs .falign
	{
		r1 = ##H2K_VM_VERSION
		memw(ME+#CONTEXT_r00) = r1.new
		jumpr r31
	}
FUNC_END H2K_vmtrap_version

FUNC_START H2K_vmtrap_return .text.vm.funcs .falign
#define SSR r1
#define GSSR_GELR r3:2
#define GSSR r3
#define GELR r2
#define GOSP r5
#define TMP0 r6
#define TMP1 r7
#define TMP2 r8
	{
		GSSR_GELR = g1:0
		SSR = memw(ME+#CONTEXT_ssr)
		TMP2 = ##~(1<<SSR_GUEST_BIT)
	}
	{
		
		p0 = cmp.gt(GSSR,#-1)			// UM is bit 31, so test for negative
		TMP1 = add(GSSR,GSSR)			// shift GSSR left 1 to get IE to MSB
		TMP0 = extractu(GSSR,#1,#H2K_GSSR_SS_BIT)
		elr = GELR
	}
	{
		SSR = insert(TMP0,#1,#SSR_SS_BIT)
		GOSP = g2
		p1 = cmp.gt(TMP1,#-1)			// IE bit is bit 30, we shifted left 1
	}
	{
		if (!p0) SSR = AND(SSR,TMP2)		//clrbit(SSR,SSR_GUEST_BIT)
		if (!p0) memw(ME+#CONTEXT_r29) = GOSP
		if (!p0) GOSP = memw(ME+#CONTEXT_r29)
	}
	.falign
1:
	{
		memw(ME+#CONTEXT_ssr) = SSR
		g2 = GOSP
		if (p1) jumpr r31
	}
	jump H2K_enable_guest_interrupts
#undef SSR
#undef GSSR_GELR
#undef GSSR
#undef GELR
#undef TMP1
#undef TMP0
#undef GOSP
FUNC_END H2K_vmtrap_return

FUNC_START H2K_vmtrap_setvec .text.vm.funcs .falign
	{
		r1 = memw(ME+#CONTEXT_r00)
		memw(ME+#CONTEXT_r00) = #0
	}
	{
		memw(ME+#CONTEXT_gevb) = r1
		jumpr r31
	}
FUNC_END H2K_vmtrap_setvec

/* Is enable while enable common? disable while disabled? No change can go faster... */
FUNC_START H2K_vmtrap_setie .text.vm.funcs .falign
	{
		r1 = memw(ME+#CONTEXT_r00)
		r2 = add(ME,#CONTEXT_atomic_status_word)
		if (tstbit(r1.new,#0)) jump:nt 2f
	}
1:
	{
		r3 = memw_locked(r2)
	}
	{
		r3 = clrbit(r3,#H2K_VMSTATUS_IE_BIT)
		r4 = extractu(r3,#1,#H2K_VMSTATUS_IE_BIT)
	}
	{
		memw_locked(r2,p0) = r3
	}
	{
		if (p0) jumpr r31
		if (p0) memw(ME+#CONTEXT_r00) = r4
	}
	jump 1b
2:
	{
		r17:16 = combine(r31,ME)
		call H2K_enable_guest_interrupts
	}
	{
		jumpr r17
		memw(r16+#CONTEXT_r00) = r0
	}
FUNC_END H2K_vmtrap_setie

FUNC_START H2K_vmtrap_getie .text.vm.funcs .falign
	{
		r1 = memw(ME+#CONTEXT_atomic_status_word)
	}
	{
		r1 = extractu(r1,#1,#H2K_VMSTATUS_IE_BIT)
		memw(ME+#CONTEXT_r00) = r1.new
		jumpr r31
	}
FUNC_END H2K_vmtrap_getie

FUNC_START H2K_vmtrap_get_pcycles .text.vm.funcs .falign
	{
		r3:2 = s31:30			// pcycles
		r1 = add(H2K_GP,#KG_oncpu_start)
		r5:4 = memd(ME+#CONTEXT_totalcycles)
		r6 = memub(ME+#CONTEXT_hthread)
	}
	{
		r5:4 = add(r5:4,r3:2)
		r3:2 = memd(r1+r6<<#3)
	}
	{
		r5:4 = sub(r5:4,r3:2)
	}
	{
		memd(ME+#CONTEXT_r0100) = r5:4
		jumpr r31
	}
FUNC_END H2K_vmtrap_get_pcycles

FUNC_START H2K_vmtrap_set_pcycles .text.vm.funcs .falign
	{
		r3:2 = s31:30
		r5:4 = memd(ME+#CONTEXT_r0100)
		r6 = memub(ME+#CONTEXT_hthread)
		r1 = add(H2K_GP,#KG_oncpu_start)
	}
	{
		memd(ME+#CONTEXT_totalcycles) = r5:4
		memd(r1+r6<<#3) = r3:2
		jumpr r31
	}
FUNC_END H2K_vmtrap_set_pcycles

FUNC_START H2K_vmtrap_wait .text.vm.funcs .falign
#define MESAV r16
#define RET r17
#define RET_MESAV r17:16
#define ID r1
#define VMBLOCK r2
#define HTHREAD r3
#define WAITING r5:4
#define NEWMASK r7:6
#define RUNLIST r8
#define RUNLIST_PRIOS r9
	k0lock
	{
		RET_MESAV = combine(r31,ME)
		call H2K_vm_do_work
	}
	{
		p0 = cmp.eq(r0,#-1)
		if (!p0.new) jump:nt 9f
		if (p0.new) ID = memw(MESAV+#CONTEXT_id)
		if (p0.new) VMBLOCK = memw(MESAV+#CONTEXT_vmblock)
	}
	{
		ID = extractu(ID,#H2K_ID_CPUIDX_BITS,#H2K_ID_RSVD_BITS)
		WAITING = memd(VMBLOCK+#VMBLOCK_waiting_cpus)
		NEWMASK = #1
		HTHREAD = HTID
	}
	{
		p0 = cmp.gt(ID,#63)
		WAITING |= asl(NEWMASK,ID)
		RUNLIST = add(H2K_GP,#KG_runlist)
		RUNLIST_PRIOS = add(H2K_GP,#KG_runlist_prios)
	}
	{
		if (!p0) memd(VMBLOCK+#VMBLOCK_waiting_cpus) = WAITING
		memb(MESAV+#CONTEXT_status) = #STATUS_VMWAIT
		r11:10 = combine(#0,#-1)
	}
	{
		jump H2K_dosched
		r1:0 = combine(HTHREAD,MESAV)
		memw(RUNLIST+HTHREAD<<#2) = r11
		memh(RUNLIST_PRIOS+HTHREAD<<#1) = r10
	}
9:
	k0unlock
	{
		memw(MESAV+#CONTEXT_r00) = r0
		jumpr RET
	}
FUNC_END H2K_vmtrap_wait

FUNC_START H2K_vmtrap_yield .text.vm.funcs .falign
	{
		memw(ME+#CONTEXT_r00) = #0
		jump H2K_sched_yield
	}
FUNC_END H2K_vmtrap_yield

FUNC_START H2K_vmtrap_start .text.vm.funcs .falign
	{
		r17:16 = combine(r31,ME)
		r5 = ME
		r3 = memub(ME+#CONTEXT_base_prio)
	}
	{
		r4 = memw(ME+#CONTEXT_vmblock)
		r1:0 = memd(ME+#CONTEXT_r0100)
		call H2K_thread_create_no_squash
		r2 = #0
	}
	{
		memw(r16+#CONTEXT_r00) = r0
		jumpr r17
	}
FUNC_END H2K_vmtrap_start

FUNC_START H2K_vmtrap_stop .text.vm.funcs .falign
	{
		r1 = ME
		r0 = memw(ME + #CONTEXT_r00)
		jump H2K_thread_stop
	}
FUNC_END H2K_vmtrap_stop

FUNC_START H2K_vmtrap_vmpid .text.vm.funcs .falign
	{
		r1 = memw(ME+#CONTEXT_id)
		memw(ME+#CONTEXT_r00) = r1.new
		jumpr r31
	}
FUNC_END H2K_vmtrap_vmpid

FUNC_START H2K_vmtrap_setregs .text.vm.funcs .falign
	{
		r3:2 = memd(ME+#CONTEXT_r0302)
		r5:4 = memd(ME+#CONTEXT_r0100)
	}
	g3:2 = r3:2
	{
		g1:0 = r5:4
		jumpr r31
	}
FUNC_END H2K_vmtrap_setregs

FUNC_START H2K_vmtrap_getregs .text.vm.funcs .falign
	r3:2 = g3:2
	{
		r5:4 = g1:0
		memd(ME+#CONTEXT_r0302) = r3:2
	}
	{
		memd(ME+#CONTEXT_r0100) = r5:4
		jumpr r31
	}
FUNC_END H2K_vmtrap_getregs

