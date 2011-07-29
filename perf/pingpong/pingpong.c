/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>

h2_sem_t toping;
h2_sem_t topong;
h2_sem_t tomain;

unsigned long long int ping_stack[64];
unsigned long long int pong_stack[64];

#ifdef DEBUG
#define ITERS 10
#else
#define ITERS (1000*1000)
#endif

void ping(void *unused) {
	int i;
	h2_sem_up(&tomain);
	for (i = 0; i < ITERS; i++) {
		h2_sem_down(&toping);
		#ifdef DEBUG
		printf("ping!\n");
		#endif
		h2_sem_up(&topong);
	}
	h2_sem_up(&tomain);
	h2_thread_stop();
}

void pong(void *unused) {
	h2_sem_up(&tomain);
	while (1) {
		h2_sem_down(&topong);
		#ifdef DEBUG
		printf("pong!\n");
		#endif
		h2_sem_up(&toping);
	}
	h2_thread_stop();
}

char context_space[1024];

int main() {
	unsigned long long int start,end;
	h2_init(NULL);
	h2_config_add_thread_storage(context_space,sizeof(context_space));
	h2_sem_init_val(&toping,0);
	h2_sem_init_val(&topong,0);
	h2_sem_init_val(&tomain,0);
	h2_thread_create((void *)ping,&ping_stack[63],NULL,254);
	h2_thread_create((void *)pong,&pong_stack[63],NULL,254);
	h2_sem_down(&tomain);
	h2_sem_down(&tomain);
	start = h2_get_core_pcycles();
	h2_sem_up(&toping);
	h2_sem_down(&tomain);
	end = h2_get_core_pcycles();
	printf("Done! cycles=%lld\n", end-start);
}

