/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <h2.h>
#include "arch_pmu.h"

h2_sem_t sema,semb;

#define THREAD_STACK_SIZE 128
unsigned long long int stack_space[THREAD_STACK_SIZE];

void worker_thread(void *param)
{
	h2_sem_up(&sema);
	h2_sem_down(&semb);
	h2_sem_up(&sema);
}

int main(int argc, char **argv)
{
	int i;

	RESET_PMU();
	//	printf("PMUEVTCFG %08x\n", h2_pmu_getreg(H2_PMUEVTCFG));
	printf("Hello, World!\n");
	for (i = 0; i < argc; i++) {
		printf("argv[%d] = <%s>\n",i,argv[i]);
	}
	h2_sem_init_val(&sema,0);
	h2_sem_init_val(&semb,0);
	if (h2_thread_create(worker_thread,&stack_space[THREAD_STACK_SIZE],0,4) == -1) {
		printf("Can't create worker thread");
	}
	h2_sem_down(&sema);
	h2_sem_up(&semb);
	h2_sem_down(&sema);
	printf("Wahoo!\n");

	DUMP_PMU();
	h2_thread_stop(0);
	return 0;
}

