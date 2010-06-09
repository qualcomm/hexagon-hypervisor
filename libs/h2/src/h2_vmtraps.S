/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_trap_constants.h"

	.text
	.global h2_vmtrap_newptb
	.global h2_vmtrap_clrmap
	.global h2_vmtrap_setvec
	.global h2_vmtrap_setregs
	.global h2_vmtrap_getregs

h2_vmtrap_setvec:
	trap1(#H2_VMTRAP_SETVEC)
	jumpr r31

h2_vmtrap_newptb:
	trap1(#H2_VMTRAP_NEWPTB)
	jumpr r31

h2_vmtrap_clrmap:
	trap1(#H2_VMTRAP_CLRMAP)
	jumpr r31

h2_vmtrap_setregs:
	trap1(#H2_VMTRAP_SETREGS)
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

