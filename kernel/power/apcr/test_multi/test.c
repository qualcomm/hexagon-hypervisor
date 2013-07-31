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

#define INTERRUPT_NUM 32
#define ITERS 1

#define PASSFAIL_PA 0x01000000
#define PASSFAIL_VA 0xE0000000

#define STACK_SIZE 128
#define TASKS 4
#define PRIO 3

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

	for (j = 0; j < ITERS; j++) {

		h2_sem_init_val(&done, 0);
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

		h2_sem_down(&done);
	}

	*sigil = 0xe0f0beef;
	h2_thread_stop(0);
}

/* void intr() { */
/* 	swi(1 << INTERRUPT_NUM); */
/* } */

int main() 
{
	H2K_mem_tlbfmt_t pte;
	unsigned long vm;
	u32_t asid;

	h2_init(NULL);

	pte.raw = 0;
	pte.ppd = ((PASSFAIL_PA >> 12) << 1) | 1;
	pte.cccc = 0x6;
	pte.xwru = 0xf;
	pte.vpn = (PASSFAIL_VA >> 12);
	pte.global = 1;
	pte.valid = 1;

	H2K_mem_tlb_write(9,pte.raw);

	/* set URWX in monitor TLB entry permissions */
	/* not really necessary to find our asid since the TLB entry will be global
		 anyway, but... */
	asm volatile (
	" %0 = ssr \n"
	" %0 = extractu(%0,#7,#8)\n" 
	: "=r"(asid));

	u32_t tlb_index = H2K_mem_tlb_probe(H2K_LINK_ADDR, asid);
	/* if (tlb_index == 0x80000000) { */
	/* 	FAIL("Can't find monitor TLB entry"); */
	/* } */
	u64_t tlb_entry = H2K_mem_tlb_read(tlb_index);
	tlb_entry |= 0xfULL << 28;
	H2K_mem_tlb_write(tlb_index, tlb_entry);

	asm volatile ("syncht");

	vm = h2_config_vmblock_init(0,SET_CPUS_INTS, TASKS + 1, 0);
	h2_config_vmblock_init(vm, SET_PMAP_TYPE, 0, 0);
	h2_config_vmblock_init(vm, SET_PRIO_TRAPMASK, 0x0, 0xffffffff);

	h2_vmboot(vmmain, &main_thread_stack[STACK_SIZE - 1], 0, 0, vm);

	/* int i = 10000; */
	/* while (i--); */
	/* intr(); */

	h2_thread_stop(0);
	return 0;
}

