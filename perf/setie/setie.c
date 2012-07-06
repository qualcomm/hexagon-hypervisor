/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <h2.h>

#define SETIE_2 h2_vmtrap_setie(0); h2_vmtrap_setie(1)
#define SETIE_8 SETIE_2; SETIE_2; SETIE_2; SETIE_2
#define SETIE_32 SETIE_8; SETIE_8; SETIE_8; SETIE_8
#define SETIE_128 SETIE_32; SETIE_32; SETIE_32; SETIE_32
#define SETIE_512 SETIE_128; SETIE_128; SETIE_128; SETIE_128
#define SETIE_2048 SETIE_512; SETIE_512; SETIE_512; SETIE_512

int main(void){
	unsigned long long start, end;
	h2_vmtrap_setie(1);
	start = h2_get_core_pcycles();
	SETIE_2048;
	end = h2_get_core_pcycles();
	h2_vmtrap_setie(0);
	printf("Average PCycles for 2 setie's (1 disable followed by 1 enable):  %.0f\n", (double) (end - start) / 1024.0);
	return 0;
}
