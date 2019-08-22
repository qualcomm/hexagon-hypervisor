/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * EJP: README NOW! 
 * 
 * With pthreads becoming the canonical interface, this kind of test
 * where we don't boot up normally and where a spawned guest shares 
 * an address space is quite ... tricky ... to make right.
 * 
 * Note that we call h2_thread_create_trap and h2_thread_stop_trap directly
 * here, this means that threads are created directly with a trap, 
 * without normal pthreads interaction, which means that mutexes and things 
 * probably won't work right.
 *
 * Tread carefully!
 */

#include <h2.h>
#include <boot.h>
#include <bootvm_entry.h>
#include <hw.h>
#include <stdio.h>

#ifdef RTL
#define ITERS 1
#ifndef INTERRUPT_NUM
#define INTERRUPT_NUM 2
#endif
#else
#define ITERS 2
#define HAVE_TIMER
#endif

#ifdef HAVE_TIMER
#define SLEEP (1000*1000*5)
#endif

#define PASSFAIL_VA 0x01000000

#define STACK_SIZE 256
#define TASKS 4
#define PRIO 3

H2K_offset_t offset = {{
		.size = BOOT_TLB_PGSIZE,
		.cccc = L1WB_L2C,
		.xwru = URWX,
		.pages = 0
	}};

unsigned long long int stacks[TASKS][STACK_SIZE];
unsigned long long int main_thread_stack[STACK_SIZE];
h2_sem_t sems[TASKS];
h2_sem_t done;

volatile unsigned int awake;

void task (void *tnum) {

	// wait for parent to get interrupt
	h2_sem_down(&sems[(int)tnum]);

	h2_atomic_add32((atomic_u32_t *)(&awake), 1);
	while (awake < TASKS) {
		h2_yield(); // let another task in to bump awake count
	}

	h2_sem_up(&done);
	h2_thread_stop_trap(0);
}

unsigned long main_ugp;
void vmmain(void *unused) {
	int i, j;
	int ret;
	unsigned int *sigil = (void *)(PASSFAIL_VA);
	asm volatile (" ugp = %0 " : :"r"(main_ugp));

#ifndef RTL
	h2_handle_errors(0);
#endif

	h2_sem_init_val(&done, 0);

	for (j = 0; j < ITERS; j++) {
		awake = 0;

		for (i = 0; i < TASKS; i++) {
			h2_sem_init_val(&sems[i], 0);
			do {
				/* may fail on subsequent iterations if previous tasks haven't finished
				 stopping yet, so keep trying */				
				ret = h2_thread_create_trap(task, &stacks[i][STACK_SIZE - 1], (void *)i, PRIO);
			} while (ret == -1);
		}

#ifdef HAVE_TIMER
		h2_nanosleep(SLEEP);
#else
		h2_intwait(INTERRUPT_NUM);
#endif
		for (i = 0; i < TASKS; i++) {
			h2_sem_up(&sems[i]);
		}

		for (i = 0; i < TASKS; i++) {
			h2_sem_down(&done);
		}
#ifndef RTL
		*sigil = j + 1;
#endif
	}

	*sigil = 0xe0f0beef;
	h2_dccleana(sigil);
#ifndef RTL
	printf("TEST PASSED\n");
	exit(0);
#endif
	h2_thread_stop_trap(0);
}

int main() 
{
	unsigned long vm;
	unsigned long tmp;
	asm volatile (" %0 = ugp " : "=r"(tmp));
	main_ugp = tmp;

	h2_hwconfig_hwthreads_mask(-1);  // start all hw threads

	vm = h2_config_vmblock_init(0,SET_CPUS_INTS, TASKS + 1, 0);
	h2_config_vmblock_init(vm, SET_PMAP_TYPE, (unsigned int)offset.raw, H2K_ASID_TRANS_TYPE_OFFSET);
	h2_config_vmblock_init(vm, SET_FENCES, (unsigned long)__bootvm_entry_point, 0xffffffff & BOOT_TLB_PAGE_MASK);
	h2_config_vmblock_init(vm, SET_PRIO_TRAPMASK, 0x0, 0xffffffff);

	h2_vmboot(vmmain, &main_thread_stack[STACK_SIZE - 1], 0, 0, vm);

	h2_thread_stop_trap(0);
	return 0;
}
