/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <readylist.h>
#include <hw.h>
#include <max.h>
#include <stdio.h>
#include <stdlib.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

typedef s32_t (*TB_handler_t)();

void H2K_event_vectors();

typedef union {
	TB_handler_t handler;
	unsigned int *uip;
	void *vp;
} typepun_t;

s32_t H2K_handle_reset()
{
	return 0;
}

s32_t H2K_handle_nmi()
{
	return 1;
}

s32_t H2K_handle_error()
{
	return 2;
}

s32_t H2K_handle_rsvd()
{
	return -1;
}

s32_t H2K_handle_tlbmissx()
{
	return 4;
}

s32_t H2K_handle_tlbmissrw()
{
	return 6;
}

s32_t H2K_handle_trap0()
{
	return 8;
}

s32_t H2K_handle_trap1()
{
	return 9;
}

s32_t H2K_handle_int()
{
	return 16;
}

s32_t testvals[] = {
	 0, 1, 2,-1, 4,-1, 6,-1, 8, 9,-1,-1,-1,-1,-1,-1,
	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
	16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16
};

int main() 
{
	s32_t i,ret;
	typepun_t base,test;
	base.vp = &H2K_event_vectors;
	for (i = 0; i < (sizeof(testvals)/sizeof(testvals[0])); i++) {
		if (testvals[i] < 0) continue;
		test.uip = base.uip + i;
		ret = test.handler();
		if (ret != testvals[i]) {
			printf("event %d: expected %d\n",i,testvals[i]);
			FAIL("Incorrect event return");
		}
	}
	puts("TEST PASSED\n");
	return 0;
}

