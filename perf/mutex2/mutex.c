/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>
#include <hexagon_protos.h>

#define N_MUTEXES 1

#define N_SPINS 300

#define POLYNOMIAL 0x800000000000000DULL

h2_mutex_t mutexes[N_MUTEXES];
unsigned int counters[N_MUTEXES];

h2_sem_t startsem,donesem;

#define N_WORKERS 12
#define STACKSIZE 128

unsigned long long int stacks[N_WORKERS][STACKSIZE];

static inline unsigned int myrand(unsigned int mod, unsigned long long int *lfsrp)
{
	unsigned long long int data = *lfsrp;
	data = Q6_P_lfs_PP(data,POLYNOMIAL);
	*lfsrp = data;
	return data % mod;
}

void FAIL(const char *msg)
{
	puts("FAIL");
	puts(msg);
	exit(1);
}

void worker(void *arg)
{
	unsigned long long int lfsr __attribute__((unused)) ;
	int i,base,count;
	lfsr = (unsigned long)arg * 1234567ULL;
	printf("Worker %x started\n",(unsigned int)arg);
	h2_sem_up(&donesem);
	h2_sem_down(&startsem);
	base = 0;
	for (i = 0; i < N_SPINS; i++) {
		h2_mutex_lock(&mutexes[base]);
		count = counters[base];
		counters[base] = count + 1;
		h2_mutex_unlock(&mutexes[base]);
	}
	h2_sem_up(&donesem);
	printf("Worker %x done\n",(unsigned int)arg);
	h2_thread_stop(0);
}

int main()
{
	int i;
	unsigned long long int start,end,count;
	unsigned int avg;
	printf("Hello!\n");
	h2_sem_init_val(&donesem,0);
	h2_sem_init_val(&startsem,0);
	for (i = 0; i < N_MUTEXES; i++) {
		h2_mutex_init(&mutexes[i]);
		counters[i] = 0;
	}
	printf("Starting Workers...\n");
	for (i = 0; i < N_WORKERS; i++) {
		h2_thread_create(worker,&stacks[i][STACKSIZE],(void *)i,100+i);
		h2_sem_down(&donesem);
	}
	printf("Workers GO!\n");
	h2_sem_add(&startsem,100);
	start = h2_get_core_pcycles();
	for (i = 0; i < N_WORKERS; i++) {
		h2_sem_down(&donesem);
	}
	end = h2_get_core_pcycles();
	printf("Cycles: %lld\n",end-start);
	count = 0;
	for (i = 0; i < N_MUTEXES; i++) {
		count += counters[i];
	}
	avg = (end-start)/count;
	printf("%lld locks/trylocks/unlocks, approx %d cycles per pair\n",count,avg);
	printf("Done!\n");
	printf("TEST PASSED avg=%d\n",avg);
	h2_thread_stop(0);
	return 0;
}

