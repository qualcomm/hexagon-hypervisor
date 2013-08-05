/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <hw.h>
#include <h2.h>
#include <h2_vm.h>
#include <tlbfmt.h>
#include <tlbmisc.h>
#include <h2_atomic.h>
#include <h2_common_pmap.h>
#include <boot.h>
#include <bootvm_entry.h>

#ifndef DEBUG
#define ITERS 1
#ifndef INTERRUPT_NUM
#define INTERRUPT_NUM 32
#endif
#else
#define ITERS 2
#ifndef INTERRUPT_NUM
#define INTERRUPT_NUM 30
#endif
#endif

#define PASSFAIL_VA 0x01000000

#define STACK_SIZE 128
#define TASKS 4
#define PRIO 3

H2K_offset_t offset = {{
		.size = BOOT_TLB_PGSIZE,
		.cccc = L1WB_L2C,
		.xwru = URWX,
		.pages = 0
	}};

u64_t stacks[TASKS][STACK_SIZE];
unsigned long long int main_thread_stack[STACK_SIZE];
h2_sem_t sems[TASKS];
h2_sem_t done;

volatile u32_t awake;

void task (void *tnum) {

	// wait for parent to get interrupt
	h2_sem_down(&sems[(int)tnum]);

	h2_atomic_add32((atomic_u32_t *)(&awake), 1);
	while (awake < TASKS) {
		h2_yield(); // let another task in to bump awake count
	}

	h2_sem_up(&done);
	h2_thread_stop(0);
}

void vmmain(void *unused) {
	int i, j;
	int ret;
	u32_t *sigil = (void *)(PASSFAIL_VA);

	h2_sem_init_val(&done, 0);

	for (j = 0; j < ITERS; j++) {
		awake = 0;

		for (i = 0; i < TASKS; i++) {
			h2_sem_init_val(&sems[i], 0);
			do {
				/* may fail on subsequent iterations if previous tasks haven't finished
				 stopping yet, so keep trying */				
				ret = h2_thread_create(task, &stacks[i][STACK_SIZE - 1], (void *)i, PRIO);
			} while (ret == -1);
		}

		h2_intwait(INTERRUPT_NUM);
		for (i = 0; i < TASKS; i++) {
			h2_sem_up(&sems[i]);
		}

		for (i = 0; i < TASKS; i++) {
			h2_sem_down(&done);
		}
#ifdef DEBUG
		*sigil = j + 1;
#endif
	}

	*sigil = 0xe0f0beef;
	h2_thread_stop(0);
}

#ifdef DEBUG
void intr() {
	swi(1 << INTERRUPT_NUM);
}
#endif

int main() 
{
	unsigned long vm;

	vm = h2_config_vmblock_init(0,SET_CPUS_INTS, TASKS + 1, 0);
	h2_config_vmblock_init(vm, SET_PMAP_TYPE, (unsigned int)offset.raw, H2K_ASID_TRANS_TYPE_OFFSET);
	h2_config_vmblock_init(vm, SET_FENCES, (unsigned long)__bootvm_entry_point, (0xffffffff >> BOOT_TLB_PGBITS) << BOOT_TLB_PGBITS);
	h2_config_vmblock_init(vm, SET_PRIO_TRAPMASK, 0x0, 0xffffffff);

	h2_vmboot(vmmain, &main_thread_stack[STACK_SIZE - 1], 0, 0, vm);

#ifdef DEBUG
	int j;
	volatile int i;
	for (j = 0; j < ITERS; j++) {
		i = 10000;
		while (i--);
		intr();
		i = 100000;
		while (i--);
	}
#endif

	h2_thread_stop(0);
	return 0;
}
