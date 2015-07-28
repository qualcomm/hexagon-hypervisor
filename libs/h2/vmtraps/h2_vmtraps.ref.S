/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"
#include "h2_trap_constants.h"

FUNC_START h2_vmtrap_return .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_RETURN)
	jumpr r31
FUNC_END h2_vmtrap_return

FUNC_START h2_vmtrap_setvec .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_SETVEC)
	jumpr r31
FUNC_END h2_vmtrap_setvec

FUNC_START h2_vmtrap_setie .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_SETIE)
	jumpr r31
FUNC_END h2_vmtrap_setie

FUNC_START h2_vmtrap_getie .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_GETIE)
	jumpr r31
FUNC_END h2_vmtrap_getie

FUNC_START h2_vmtrap_intop .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_INTOP)
	jumpr r31
FUNC_END h2_vmtrap_intop

FUNC_START h2_vmtrap_clrmap .text.h2.vmtrap .p2align 2
	r1 = #1
	trap1(#H2_VMTRAP_CLRMAP)
	jumpr r31
FUNC_END h2_vmtrap_clrmap

FUNC_START h2_vmtrap_newmap .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_NEWMAP)
	jumpr r31
FUNC_END h2_vmtrap_newmap

FUNC_START h2_vmtrap_cachectl .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_CACHECTL)
	jumpr r31
FUNC_END h2_vmtrap_cachectl

FUNC_START h2_vmtrap_get_pcycles .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_GET_PCYCLES)
	jumpr r31
FUNC_END h2_vmtrap_get_pcycles

FUNC_START h2_vmtrap_set_pcycles .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_SET_PCYCLES)
	jumpr r31
FUNC_END h2_vmtrap_set_pcycles

FUNC_START h2_vmtrap_wait .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_WAIT)
	jumpr r31
FUNC_END h2_vmtrap_wait

FUNC_START h2_vmtrap_yield .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_YIELD)
	jumpr r31
FUNC_END h2_vmtrap_yield

FUNC_START h2_vmtrap_start .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_START)
	jumpr r31
FUNC_END h2_vmtrap_start

FUNC_START h2_vmtrap_stop .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_STOP)
	jumpr r31
FUNC_END h2_vmtrap_stop

FUNC_START h2_vmtrap_vmpid .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_VMPID)
	jumpr r31
FUNC_END h2_vmtrap_vmpid

FUNC_START h2_vmtrap_setregs .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_SETREGS)
	jumpr r31
FUNC_END h2_vmtrap_setregs

FUNC_START h2_vmtrap_timerop .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_TIMEROP)
	jumpr r31
FUNC_END h2_vmtrap_timerop

FUNC_START h2_vmtrap_pmuctrl .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_PMUCTRL)
	jumpr r31
FUNC_END h2_vmtrap_pmuctrl

FUNC_START h2_vmtrap_info .text.h2.vmtrap .p2align 2
	trap1(#H2_VMTRAP_INFO)
	jumpr r31
FUNC_END h2_vmtrap_info

FUNC_START h2_vmtrap_getregs .text.h2.vmtrap .p2align 2
	allocframe(#8)
	{
		memd(r29+#0) = r17:16
		r17:16 = r1:0
	}
	trap1(#H2_VMTRAP_GETREGS)
	memd(r16+#0) = r1:0
	memd(r16+#8) = r3:2
	r17:16 = memd(r29+#0)
	deallocframe
	jumpr r31
FUNC_END h2_vmtrap_getregs

