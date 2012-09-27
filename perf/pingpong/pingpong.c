/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <blast.h>
#include <stdio.h>

#define MAX_THREADS 8

blast_sem_t pingsem[MAX_THREADS];
blast_sem_t tomain;

unsigned long long int stacks[MAX_THREADS][64];

#ifdef DEBUG
#define ITERS 10
#else
#define ITERS (500)
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

void ping(void *id) {
	int i;
	int myid = (int)id;
	printf("Ping thread %d\n",myid);
	blast_sem_t *in, *out;
	in = &pingsem[myid];
	if (myid == (MAX_THREADS-1)) out = &pingsem[0];
	else out = &pingsem[myid+1];
	blast_sem_up(&tomain);
	for (i = 0; i < ITERS; i++) {
		blast_sem_down(in);
		#ifdef DEBUG
		printf("%d: ping!\n",myid);
		#endif
		blast_sem_up(out);
	}
	if (myid == MAX_THREADS-1) blast_sem_up(&tomain);
	blast_thread_stop();
}

char context_space[1024];

int main() {
	unsigned long long int start,end;
	int i;
	blast_sem_init_val(&tomain,0);
	for (i = 0; i < MAX_THREADS; i++) {
		blast_sem_init_val(&pingsem[i],0);
		my_thread_create(ping,&stacks[i][64],64*8,(void *)i,100);
		blast_sem_down(&tomain);
	}
	start = blast_get_core_pcycles();
	blast_sem_up(&pingsem[0]);
	blast_sem_down(&tomain);
	end = blast_get_core_pcycles();
	printf("TEST PASSED - %.0f\n", (float) (end - start) / (float) (ITERS*MAX_THREADS));
	return 0;
}
