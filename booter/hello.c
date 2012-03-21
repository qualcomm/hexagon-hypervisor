/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <h2.h>

h2_sem_t sema,semb;

#define THREAD_STACK_SIZE 128
unsigned long long int stack_space[THREAD_STACK_SIZE];

void worker_thread(void *param)
{
	while (1) {
		h2_sem_up(&sema);
		h2_sem_down(&semb);
	}
}

int main()
{
	printf("Hello, World!\n");
	h2_sem_init_val(&sema,0);
	h2_sem_init_val(&sema,0);
	if (h2_thread_create(worker_thread,&stack_space[THREAD_STACK_SIZE],0,4) == -1) {
		printf("Can't create worker thread");
	}
	h2_sem_down(&sema);
	h2_sem_up(&semb);
	h2_sem_down(&sema);
	printf("Wahoo!\n");
}

