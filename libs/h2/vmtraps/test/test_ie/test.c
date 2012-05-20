/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <context.h>
#include <max.h>
#include <h2.h>

#define info(...) { h2_printf("INFO:  "); h2_printf(__VA_ARGS__);}
#define warn(...) { h2_printf("WARNING:  "); h2_printf(__VA_ARGS__);}
#define debug(...) { h2_printf("DEBUG:  "); h2_printf(__VA_ARGS__);}
#define error(...) { h2_printf("ERROR:  "); h2_printf(__VA_ARGS__); FAIL("");}

void FAIL(const char *str)
{
	puts("FAIL");
	h2_printf(str);
	exit(1);
}

int main() {

	unsigned int ie;

	ie = h2_vmtrap_getie();
	if (ie) {
		FAIL("Interrupts should be disabled initially");
	}

	h2_vmtrap_setie(1);
	ie = h2_vmtrap_getie();
	if (ie == 0) {
		FAIL("Interrupts should be enabled");
	}

	h2_vmtrap_setie(0);
	ie = h2_vmtrap_getie();
	if (ie) {
		FAIL("Interrupts should be disabled");
	}

	info("TEST PASSED\n");
	return(0);

}
