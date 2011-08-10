/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast.h>
#include <stdio.h>

#define NMUTEXES 192  // max supported by qurt config

int main(int argc, char *argv[]) {

	unsigned long long int start,end;
	blast_mutex_t lock[NMUTEXES];
	unsigned int count = NMUTEXES;
	unsigned int i;

	//	sscanf(argv[1], "%u", &count);
	printf("iterations = %u\n", count);

#ifdef H2_H
	h2_init(NULL);
#endif
	start = blast_get_core_pcycles();

	for (i = 0; i < count; i++) {
		blast_mutex_init(&lock[i]);
	}
	
	for (i = 0; i < count; i++) {
		blast_mutex_destroy(&lock[i]);
	}
	
	end = blast_get_core_pcycles();
	printf("cycles = %lld\n", end-start);
}

