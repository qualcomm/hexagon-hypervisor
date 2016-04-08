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
	void *tcm;

	h2_galloc_t alloc0, alloc1;

//	h2_init(0x0);

	ptr = h2_memalign(32,100);

	info("ptr_value = 0x%08x\n",ptr);
	if (ptr == NULL) {
		error("memalign\n");
	}

	tcm = (void *)h2_info(INFO_TCM_BASE);

	h2_galloc_init(&alloc0, (unsigned int)tcm, 1000, &alloc1);
	h2_galloc_init(&alloc1, (unsigned int)tcm + 1000, 2000, NULL);

	ptr = h2_galloc(&alloc0, 100, 0);
	if (ptr != tcm) {
		error("galloc 1\n");
	}

	ptr = h2_galloc(&alloc0, 901, 0);
	if (NULL != ptr) {
		error("galloc 2\n");
	}
	
	ptr = h2_galloc(&alloc0, 901, 1);
	if ((unsigned int)ptr != (unsigned int)tcm + 1000) {
		error("galloc 3\n");
	}

	h2_galloc_reset(&alloc0, 0);
	if (alloc0.cur != alloc0.base) {
		error("galloc 4\n");
	}
	if (alloc1.cur == alloc1.base) {
		error("galloc 5\n");
	}

	h2_galloc_reset(&alloc0, 1);
	if (alloc1.cur != alloc1.base) {
		error("galloc 6\n");
	}

	ptr = h2_galloc(&alloc0, 3000, 1);
	if (NULL != ptr) {
		error("galloc 7\n");
	}

	info("TEST PASSED\n");

	h2_thread_stop(0);
	return(0);
}
