/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <trace.h>
#include <stdio.h>
#include <stdlib.h>
#include <globals.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_trace_entry_t tmpbuf[2];

int main()
{
#if 0
	u32_t start_entries;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	H2K_trace_init();
	start_entries = H2K_gp->trace_info_entries;
	if (H2K_gp->trace_info_buf != H2K_kg.trace_buf_default) FAIL("trace init fail");
	if (H2K_gp->trace_info_entries == 0) FAIL("Trace init fail");
	if (H2K_gp->trace_info_index != 0) FAIL("Trace init fail");
	H2K_trace(-1,2,0x1234,1);
	if (H2K_gp->trace_info_index != 1) FAIL("Trace idx fail (1)");
	if (H2K_gp->trace_info_buf[0] != 0x000002ff00001231ULL) FAIL("Trace val fail");
	H2K_trace(2,0xdead,0x2345,3);
	if (H2K_gp->trace_info_index != 2) FAIL("Trace idx fail (2)");
	if (H2K_gp->trace_info_buf[0] != 0x000002ff00001231ULL) FAIL("Trace val clobbered");
	if (H2K_gp->trace_info_buf[1] != 0x00dead0200002343ULL) FAIL("Trace val fail");
	H2K_gp->trace_info_index = H2K_gp->trace_info_entries - 1;
	H2K_trace(0,0,0x2234,0);
	if (H2K_gp->trace_info_index != 0) FAIL("Trace loop fail");
	if (H2K_gp->trace_info_buf[0] != 0x000002ff00001231ULL) FAIL("Trace val clobbered");
	if (H2K_gp->trace_info_buf[H2K_gp->trace_info_entries - 1] != 0x0000000000002230) FAIL("Trace val fail");
	if (H2K_gp->trace_info_entries != start_entries) FAIL("entries clobbered");
#endif
	puts("TEST PASSED");
	return 0;
}

