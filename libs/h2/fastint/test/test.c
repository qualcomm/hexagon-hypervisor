/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2.h>

#define info(...) { h2_printf("INFO:  "); h2_printf(__VA_ARGS__);}
#define warn(...) { h2_printf("WARNING:  "); h2_printf(__VA_ARGS__);}
#define debug(...) { h2_printf("DEBUG:  "); h2_printf(__VA_ARGS__);}
#define error(...) { h2_printf("ERROR:  "); h2_printf(__VA_ARGS__); FAIL("");}

void FAIL(const char *str)
{
	exit(1);
}

#if __QDSP6_ARCH__ <= 3
#define TEST_INT	31
#else
#define TEST_INT	0
#endif

#define TEST_SIGMASK	0x1
#define TEST_THREADS	3

#define SIGNAL_CNT	20
#define WDOG_TIMEOUT	300

//  Should have a watchdog (main) thread, a timer thread, and a signal watcher thread

unsigned long context_space[1024];
unsigned long stack_space[TEST_THREADS][1024]; 

volatile unsigned long count = 0;

h2_anysignal_t	int_sig;
h2_mutex_t	mutex;
h2_sem_t	sem;

int int2sig(int intnum) 
{
	h2_anysignal_set(&int_sig,TEST_SIGMASK);  
	//h2_mutex_unlock(&mutex);
	//h2_sem_add(&sem,1);
	return 1;
}

void timer(int dummy)
{
	int i,j;
	info("Timer started\n");

	for (i=0; i<SIGNAL_CNT; i++) {
		//info("Sending interrupt\n");
		for (j=0; j<1000; j++) {
			asm volatile("nop;");
		}
		asm volatile("R0 = #1; swi(R0);":::"r0");
		puts("tick");
	}
	h2_thread_stop();  //  weird things happen without this here.
}

void watcher(int dummy)
{
	info("Watcher sterted\n");

	while (1) {
		h2_anysignal_wait(&int_sig, TEST_SIGMASK);
		h2_anysignal_clear(&int_sig, TEST_SIGMASK);
		//h2_sem_down(&sem);
		//h2_mutex_lock(&mutex);
		count++;
		puts("Got signal");
	}
}

int main()
{
	int i,j;

	h2_init(0);
	h2_config_add_thread_storage(context_space,sizeof(context_space));
	info("Starting\n");

	h2_anysignal_init(&int_sig);
	h2_anysignal_clear(&int_sig, TEST_SIGMASK);

	h2_mutex_init(&mutex);
	h2_mutex_lock(&mutex);

	h2_sem_init(&sem);
	h2_sem_down(&sem);

	h2_register_fastint(TEST_INT,int2sig);
	h2_thread_create((void *)timer,&stack_space[2],0xffffffff,0);
	h2_thread_create((void *)watcher,&stack_space[1],0xffffffff,0);  // stackgrowsup

	for (i=0; i<WDOG_TIMEOUT; i++) {
		for (j=0; j<1000; j++) {
			asm volatile("nop;");
		}
	}

	info("Checking results\n");

	if (count != SIGNAL_CNT) {
		error("Wrong signal count\n");
	}
	else {
		info("TEST PASSED\n");
	}

	return 0;

}

