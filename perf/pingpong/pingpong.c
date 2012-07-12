/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast.h>
#include <stdio.h>

blast_sem_t toping;
blast_sem_t topong;
blast_sem_t tomain;

unsigned long long int ping_stack[64];
unsigned long long int pong_stack[64];

#ifdef DEBUG
#define ITERS 10
#else
#define ITERS (1000)
#endif

#ifdef H2_H
static inline void my_thread_create(void *f, void *s, int ss, void *p, int prio) { h2_thread_create(f,s,p,prio); }
#else
static inline void my_thread_create(void *f, void *s, int ss, void *p, int prio) { 
	qurt_thread_attr_t attr;
	qurt_thread_t tid;
	int ret;
	char tname[16];
	sprintf(tname, "T%08x",(int)p);

	//printf( "thread %d started\n", id );
	qurt_thread_attr_set_name(&attr, (char*)tname);
    
	qurt_thread_attr_init (&attr);
	qurt_thread_attr_set_stack_size (&attr, ss);
	qurt_thread_attr_set_stack_addr (&attr, s);
	qurt_thread_attr_set_priority (&attr, prio);
	/* TIMETEST IDs 0 - 31 are reserved for interrrupts */
	qurt_thread_attr_set_timetest_id (&attr, prio);

	ret = qurt_thread_create(&tid, &attr, f, (void *)p);
	if (ret == -1) {
		printf(" failed to create thread \n");
	}
}
#endif

void ping(void *unused) {
	int i;
	blast_sem_up(&tomain);
	for (i = 0; i < ITERS; i++) {
		blast_sem_down(&toping);
		#ifdef DEBUG
		printf("ping!\n");
		#endif
		blast_sem_up(&topong);
	}
	blast_sem_up(&tomain);
	blast_thread_stop();
}

void pong(void *unused) {
	blast_sem_up(&tomain);
	while (1) {
		blast_sem_down(&topong);
		#ifdef DEBUG
		printf("pong!\n");
		#endif
		blast_sem_up(&toping);
	}
	blast_thread_stop();
}

char context_space[1024];

int main() {
	unsigned long long int start,end;
	blast_sem_init_val(&toping,0);
	blast_sem_init_val(&topong,0);
	blast_sem_init_val(&tomain,0);
	my_thread_create((void *)ping,&ping_stack[64],64*8,NULL,254);
	my_thread_create((void *)pong,&pong_stack[64],64*8,NULL,254);
	blast_sem_down(&tomain);
	blast_sem_down(&tomain);
	start = blast_get_core_pcycles();
	blast_sem_up(&toping);
	blast_sem_down(&tomain);
	end = blast_get_core_pcycles();
	printf("TEST PASSED - %.0f\n", (float) (end - start) / (float) ITERS);
	return 0;
}
