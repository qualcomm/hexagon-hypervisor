/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include "h2_common_asm_std.h"

	.global h2_context_save
FUNC_START h2_context_save .text.h2.context .p2align 5
	{
		memd(r29+#168) = r27:26
		memd(r29+#160) = r25:24
		r27:26 = g3:2
	}
	{
		memd(r29+#152) = r23:22
		memd(r29+#144) = r21:20
		r25:24 = g1:0
	}
	{
		memd(r29+#136) = r19:18
		memd(r29+#128) = r17:16
		r23:22 = c13:12
	}
	{
		memd(r29+#120) = r15:14
		memd(r29+#112) = r13:12
		r21:20 = c11:10
	}
	{
		memd(r29+#104) = r11:10
		memd(r29+#96) = r9:8
		r19:18 = c7:6
	}
	{
		memd(r29+#88) = r7:6
		memd(r29+#80) = r5:4
		r17 = usr
	}
	{
		memd(r29+#72) = r3:2
		memd(r29+#64) = r1:0
		r16 = p3:0
	}
	{
		memd(r29+#56) = r27:26
		memd(r29+#48) = r25:24
		r15:14 = c3:2
	}
	{
		memd(r29+#40) = r23:22
		memd(r29+#32) = r21:20
		r13:12 = c1:0
	}
	{
		memd(r29+#24) = r19:18
		memd(r29+#16) = r17:16
	}
	{
		memd(r29+#8) = r15:14
		memd(r29+#0) = r13:12
		callr r31
	}
	.falign
.Lcontext_restore:
	{
		r13:12 = memd(r29+#0)
		r15:14 = memd(r29+#8)
	}
	{
		r17:16 = memd(r29+#16)
		r19:18 = memd(r29+#24)
		c1:0 = r13:12
	}
	{
		r21:20 = memd(r29+#32)
		r23:22 = memd(r29+#40)
		c3:2 = r15:14
	}
	{
		r25:24 = memd(r29+#48)
		r27:26 = memd(r29+#56)
		p3:0 = r16
	}
	{
		r1:0 = memd(r29+#64)
		r3:2 = memd(r29+#72)
		usr = r17
	}
	{
		r5:4 = memd(r29+#80)
		r7:6 = memd(r29+#88)
		c7:6 = r19:18
	}
	{
		r9:8 = memd(r29+#96)
		r11:10 = memd(r29+#104)
		c11:10 = r21:20
	}
	{
		r13:12 = memd(r29+#112)
		r15:14 = memd(r29+#120)
		c13:12 = r23:22
	}
	{
		r17:16 = memd(r29+#128)
		r19:18 = memd(r29+#136)
		g1:0 = r25:24
	}
	{
		r21:20 = memd(r29+#144)
		r23:22 = memd(r29+#152)
		g3:2 = r27:26
	}
	{
		r25:24 = memd(r29+#160)
		r27:26 = memd(r29+#168)
		jumpr r31
	}
FUNC_END h2_context_save

