/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>

#define info(...) { h2_printf("INFO:  "); h2_printf(__VA_ARGS__);}
#define warn(...) { h2_printf("WARNING:  "); h2_printf(__VA_ARGS__);}
#define debug(...) { h2_printf("DEBUG:  "); h2_printf(__VA_ARGS__);}
#define error(...) { h2_printf("ERROR:  "); h2_printf(__VA_ARGS__); FAIL("");}

void FAIL(const char *str)
{
        h2_printf(str);
        exit(1);
}

int main() 
{
	void *ptr;
	h2_init(0x0);

	ptr = h2_memalign(32,100);

	info("ptr_value = 0x%08x\n",ptr);
	if (ptr == NULL) {
		error("FAIL\n");
	}
	else {
		info("TEST PASSED\n");
	}
	return(0);
}
