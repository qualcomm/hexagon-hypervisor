/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#if 0
	.data
appdata:
	.incbin "testapp.bin"
appdata_end:
	.byte 0

	.text
	.global t32_test_load
t32_test_load:
	r0 = add(pc,##appdata@PCREL)
	r1 = ##appdata_end-appdata
	loop0(1f,r1)
	r2 = r0
	r3 = ##0x01000000
	r0 = r3
	.falign
1:
	{
		r4 = memub(r2++#1)
		memb(r3++#1) = r4.new
	}:endloop0

	jumpr r31

#endif

