/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>
#include <stdio.h>
#include <hexagon_protos.h>

#define N_MUTEXES 16

#define MAX_RAND_MUTEX_TRIES 4
#define SPINRAND 16
#define BASESPINS 64

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

void spin(int randint)
{
	int i;
	for (i = 0; i < BASESPINS+randint; i++) {
		asm volatile ("nop");
	}
}

void lock_some_more(unsigned long long int *lfsrp, int base)
{
	int newbase;
	int count;
	if (base>=(N_MUTEXES-1)) return spin(myrand(SPINRAND,lfsrp));
	while ((newbase=myrand(N_MUTEXES,lfsrp)) <= base) /* find a new random */
	if (h2_mutex_trylock(&mutexes[newbase]) == 0) {
		count = counters[newbase];
		lock_some_more(lfsrp,newbase);
		counters[newbase] = count + 1;
		h2_mutex_unlock(&mutexes[newbase]);
	} else {
		spin(myrand(SPINRAND,lfsrp));
	}
}

void worker(void *arg)
{
	unsigned long long int lfsr;
	int i,j,base,count,thismax;
	lfsr = (unsigned long)arg * 1234567ULL;
	printf("Worker %x started\n",(unsigned int)arg);
	h2_sem_down(&startsem);
	for (i = 0; i < N_SPINS; i++) {
		thismax = myrand(MAX_RAND_MUTEX_TRIES,&lfsr);
		for (j = 0; j < thismax; j++) {
			base = myrand(N_MUTEXES,&lfsr);
			h2_mutex_lock(&mutexes[base]);
			count = counters[base];
			lock_some_more(&lfsr,base);
			counters[base] = count + 1;
			h2_mutex_unlock(&mutexes[base]);
		}
	}
	h2_sem_up(&donesem);
	printf("Worker %x done\n",(unsigned int)arg);
	h2_thread_stop(0);
}

int main()
{
	int i;
	int id;
	unsigned long long int start,end,count,adj;
	unsigned int avg;
	FILE *threadmap;
	printf("Hello!\n");
	if ((threadmap = fopen("threadmap.py","w")) == NULL) {
		printf("Can't open threadmap.py output\n");
		exit(1);
	}
	h2_sem_init_val(&donesem,0);
	h2_sem_init_val(&startsem,0);
	for (i = 0; i < N_MUTEXES; i++) {
		h2_mutex_init(&mutexes[i]);
		counters[i] = 0;
	}
	printf("Starting Workers...\n");
	fprintf(threadmap,"{\n");
	fprintf(threadmap," 0x%08x : 'main',\n",h2_thread_myid());
	for (i = 0; i < N_WORKERS; i++) {
		id = h2_thread_create(worker,&stacks[i][STACKSIZE],(void *)i,100+i);
		fprintf(threadmap," 0x%08x : 'worker%d',\n",id,i);
	}
	fprintf(threadmap,"}\n");
	fclose(threadmap);
	spin(10000);
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
	adj = (BASESPINS+SPINRAND/2)*N_WORKERS;
	avg = (end-start-adj)/count;
	printf("%lld locks/trylocks/unlocks, approx %d cycles per pair\n",count,avg);
	printf("Done!\n");
	printf("TEST PASSED - %d\n",avg);
	h2_thread_stop(0);
	return 0;
}

