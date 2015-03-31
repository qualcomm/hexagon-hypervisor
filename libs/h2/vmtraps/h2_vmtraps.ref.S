/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

	.text
	.global h2_vmtrap_return
	.global h2_vmtrap_setvec
	.global h2_vmtrap_setie
	.global h2_vmtrap_getie
	.global h2_vmtrap_intop
	.global h2_vmtrap_clrmap
	.global h2_vmtrap_newmap
	.global h2_vmtrap_cachectl
	.global h2_vmtrap_get_pcycles
	.global h2_vmtrap_set_pcycles
	.global h2_vmtrap_wait
	.global h2_vmtrap_yield
	.global h2_vmtrap_start
	.global h2_vmtrap_stop
	.global h2_vmtrap_vmpid
	.global h2_vmtrap_setregs
	.global h2_vmtrap_getregs
	.global h2_vmtrap_timerop
	.global h2_vmtrap_pmuctrl
	.global h2_vmtrap_info

h2_vmtrap_return:
	trap1(#H2_VMTRAP_RETURN)
	jumpr r31

h2_vmtrap_setvec:
	trap1(#H2_VMTRAP_SETVEC)
	jumpr r31

h2_vmtrap_setie:
	trap1(#H2_VMTRAP_SETIE)
	jumpr r31

h2_vmtrap_getie:
	trap1(#H2_VMTRAP_GETIE)
	jumpr r31

h2_vmtrap_intop:
	trap1(#H2_VMTRAP_INTOP)
	jumpr r31

h2_vmtrap_clrmap:
	r1 = #1
	trap1(#H2_VMTRAP_CLRMAP)
	jumpr r31

h2_vmtrap_newmap:
	trap1(#H2_VMTRAP_NEWMAP)
	jumpr r31

h2_vmtrap_cachectl:
	trap1(#H2_VMTRAP_CACHECTL)
	jumpr r31

h2_vmtrap_get_pcycles:
	trap1(#H2_VMTRAP_GET_PCYCLES)
	jumpr r31

h2_vmtrap_set_pcycles:
	trap1(#H2_VMTRAP_SET_PCYCLES)
	jumpr r31

h2_vmtrap_wait:
	trap1(#H2_VMTRAP_WAIT)
	jumpr r31

h2_vmtrap_yield:
	trap1(#H2_VMTRAP_YIELD)
	jumpr r31

h2_vmtrap_start:
	trap1(#H2_VMTRAP_START)
	jumpr r31

h2_vmtrap_stop:
	trap1(#H2_VMTRAP_STOP)
	jumpr r31

h2_vmtrap_vmpid:
	trap1(#H2_VMTRAP_VMPID)
	jumpr r31

h2_vmtrap_setregs:
	trap1(#H2_VMTRAP_SETREGS)
	jumpr r31

h2_vmtrap_timerop:
	trap1(#H2_VMTRAP_TIMEROP)
	jumpr r31

h2_vmtrap_pmuctrl:
	trap1(#H2_VMTRAP_PMUCTRL)
	jumpr r31

h2_vmtrap_info:
	trap1(#H2_VMTRAP_INFO)
	jumpr r31

h2_vmtrap_getregs:
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

