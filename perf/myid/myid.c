/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <h2.h>

#define MYID h2_thread_myid()
#define MYID_4 MYID; MYID; MYID; MYID
#define MYID_16 MYID_4; MYID_4; MYID_4; MYID_4
#define MYID_64 MYID_16; MYID_16; MYID_16; MYID_16
#define MYID_256 MYID_64; MYID_64; MYID_64; MYID_64
#define MYID_1024 MYID_256; MYID_256; MYID_256; MYID_256

int main(void){
	unsigned long long start, end;
	start = h2_get_core_pcycles();
	MYID_1024;
	end = h2_get_core_pcycles();
	printf("Average PCycles for h2_thread_myid():  %.0f\n", (double) (end - start) / 1024.0);
	printf("TEST PASSED - %.0f\n", (double) (end - start) / 1024.0);
	return 0;
}
