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

extern H2K_trace_entry_t H2K_trace_buf_default[];

H2K_trace_entry_t tmpbuf[2];

int main()
{
	u32_t start_entries;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	H2K_trace_init();
	start_entries = H2K_gp->trace_info.entries;
	if (H2K_gp->trace_info.buf != H2K_trace_buf_default) FAIL("trace init fail");
	if (H2K_gp->trace_info.entries == 0) FAIL("Trace init fail");
	if (H2K_gp->trace_info.index != 0) FAIL("Trace init fail");
	if (H2K_gp->trace_info.last_pcyclelo) FAIL("Trace init fil (pcyclelo)");
	H2K_gp->trace_info.max_trace_level = 0;
	H2K_trace(-3,2,1,0x1234);
	if (H2K_gp->trace_info.index != 1) FAIL("Trace idx fail");
	if (H2K_gp->trace_info.buf[0].raw != 0xd2011234U) FAIL("Trace val fail");
	if (H2K_gp->trace_info.last_pcyclelo != 0x1234) FAIL("Pcyclelo update");
	H2K_trace(-2,4,0xab,0x1235);
	if (H2K_gp->trace_info.index != 2) FAIL("Trace idx fail (2)");
	if (H2K_gp->trace_info.buf[0].raw != 0xd2011234U) FAIL("Trace val clobbered");
	if (H2K_gp->trace_info.buf[1].raw != 0xe4ab0001U) FAIL("Trace val fail");
	if (H2K_gp->trace_info.last_pcyclelo != 0x1235) FAIL("pcyclelo update");
	H2K_trace(-4,5,0xde,0x1234);
	if (H2K_gp->trace_info.index != 3) FAIL("Trace idx fail (2)");
	if (H2K_gp->trace_info.buf[0].raw != 0xd2011234U) FAIL("Trace val clobbered");
	if (H2K_gp->trace_info.buf[1].raw != 0xe4ab0001U) FAIL("Trace val clobbered");
	if (H2K_gp->trace_info.buf[2].raw != 0xc5deffffU) FAIL("Trace val fail");
	if (H2K_gp->trace_info.last_pcyclelo != 0x1234) FAIL("pcyclelo update");
	H2K_gp->trace_info.index = H2K_gp->trace_info.entries - 1;
	H2K_trace(0,0,0,0x2234);
	if (H2K_gp->trace_info.index != 0) FAIL("Trace loop fail");
	if (H2K_gp->trace_info.buf[0].raw != 0xd2011234U) FAIL("Trace val clobbered");
	if (H2K_gp->trace_info.buf[H2K_gp->trace_info.entries - 1].raw != 0x00001000) FAIL("Trace val fail");
	if (H2K_gp->trace_info.entries != start_entries) FAIL("entries clobbered");
	if (H2K_gp->trace_info.last_pcyclelo != 0x2234) FAIL("pcyclelo");
	H2K_gp->trace_info.buf = tmpbuf;
	H2K_gp->trace_info.entries = 2;
	H2K_trace(-1,9,0xff,0x3234);
	if (H2K_gp->trace_info.index != 1) FAIL("Trace loop fail");
	if (H2K_gp->trace_info.buf[0].raw != 0xf9ff1000U) FAIL("Trace val fail");
	if (H2K_gp->trace_info.entries != 2) FAIL("entries clobbered");
	if (H2K_gp->trace_info.last_pcyclelo != 0x3234) FAIL("pcyclelo");
	H2K_gp->trace_info.index = 0;
	H2K_trace(1,5,0x55,0x1234);
	if (H2K_gp->trace_info.index != 0) FAIL("Should not have traced");
	if (H2K_gp->trace_info.buf[0].raw != 0xf9ff1000U) FAIL("Should not have traced");
	if (H2K_gp->trace_info.entries != 2) FAIL("Should not have traced");
	if (H2K_gp->trace_info.last_pcyclelo != 0x3234) FAIL("should not have traced");
	H2K_gp->trace_info.max_trace_level = 1;
	H2K_trace(1,5,0x55,0x1234);
	if (H2K_gp->trace_info.index != 1) FAIL("Should have traced");
	if (H2K_gp->trace_info.buf[0].raw != 0x1555e000U) FAIL("Should have traced");
	if (H2K_gp->trace_info.entries != 2) FAIL("Should have traced");
	if (H2K_gp->trace_info.last_pcyclelo != 0x1234) FAIL("should have traced");
	puts("TEST PASSED");
	return 0;
}

