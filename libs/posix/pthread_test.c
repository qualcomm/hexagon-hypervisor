/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
sem_t sem;

void *pthread_test(void *arg)
{
	printf("Hello, world!\n");
	pthread_mutex_lock(&lock);
	printf("in child, id=%x arg=%p\n",pthread_self(),arg);
	pthread_mutex_unlock(&lock);
	sem_post(&sem);
	return NULL;
}

int main() {
	pthread_t child = 0xdeadbeef;
	struct timespec mytime;
	qurt_init();
	sem_init(&sem,0,0);
	pthread_mutex_lock(&lock);
	if ((pthread_create(&child,NULL,pthread_test,(void *)0xcafebabe)) != 0) {
		printf("pthread_create failed\n");
	}
	printf("started child, should be %x\n",child);
	pthread_mutex_unlock(&lock);
	do { 
		clock_gettime(CLOCK_MONOTONIC,&mytime);
		pthread_mutex_lock(&lock);
		printf("clock: sec=%ld nsec=%ld\n",mytime.tv_sec,mytime.tv_nsec);
		pthread_mutex_unlock(&lock);
	} while (mytime.tv_sec < 1);
	sem_wait(&sem);
}
