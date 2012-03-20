/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <context.h>
#include <stdlib.h>
#include <stdio.h>
#include <h2.h>
#include <tlbfmt.h>
#include <tlbmisc.h>

/*
 * This test checks the following functionality:
 * Futex wait on invalid locks
 * Futex resume on invalid locks
 * PI Mutex Lock on Invalid Locks
 * PI Mutex Unlock on Invalid Locks
 */

#define BADBASE 0xa0000000
#define PERMS(X) (0x90000000 | ((X) << 24))

h2_sem_t sema;
h2_sem_t semb;

void FAIL(const char *str)
{
	printf("FAIL: %s\n",str);
	exit(1);
}

void test_ptr_all(void *ptr)
{
	if (h2_futex_wait(ptr,0) >= 0) FAIL("Wait returned success");
	if (h2_futex_wake(ptr,1) >= 0) FAIL("Wake returned success");
	if (h2_futex_lock_pi(ptr) >= 0) FAIL("Lock returned success");
	if (h2_futex_unlock_pi(ptr) >= 0) FAIL("Unlock returned success");
}

void test_ptr_w(void *ptr)
{
	if (h2_futex_lock_pi(ptr) >= 0) FAIL("Lock returned success");
	if (h2_futex_unlock_pi(ptr) >= 0) FAIL("Unlock returned success");
}

void test_ptr_rok(void *ptr)
{
	if (h2_futex_wait(ptr,0) >= 0) FAIL("Wait returned success?");
	if (h2_futex_wake(ptr,1) > 0) FAIL("Wake woke thread?");
}

#if ARCHV <= 3
static H2K_mem_tlbfmt_t make_entry(u32_t va, u32_t pa, u32_t size, u32_t perms, u32_t asid)
{
	H2K_mem_tlbfmt_t ret;
	ret.raw = 0;
	ret.xwr = perms >> 1;
	ret.guestonly = ~(perms & 1);
	ret.asid = asid;
	ret.size = size;
	ret.ppn = pa >> 12;
	ret.vpn = va >> 12;
	ret.valid = 1;
	return ret;
}
#else
static H2K_mem_tlbfmt_t make_entry(u32_t va, u32_t pa, u32_t size, u32_t perms, u32_t asid)
{
	H2K_mem_tlbfmt_t ret;
	ret.raw = 0;
	pa >>= (12+size);
	pa = 1 + (pa << 1);
	pa <<= size;
	ret.ppd = pa;
	ret.vpn = va >> 12;
	ret.xwru = perms;
	ret.asid = asid;
	ret.valid = 1;
	return ret;
}
#endif

unsigned long long int context_space[512];

#define THREAD_STACK_SIZE 512
#define NUM_TOTAL_THREADS 5

unsigned long long int stack_space[THREAD_STACK_SIZE];
unsigned long long int main_thread_stack[THREAD_STACK_SIZE];

void worker_thread(void *param)
{
	h2_sem_t *my_sema = (void *)(PERMS(7) | (u32_t)(&sema));
	h2_sem_t *my_semb = (void *)(PERMS(7) | (u32_t)(&semb));
	while (1) {
		h2_sem_up(my_sema);
		h2_sem_down(my_semb);
	}
}

int vmmain()
{
	int i;
	printf("VM Main\n");
	h2_sem_init_val(&sema,0);
	h2_sem_init_val(&semb,0);
	if (h2_thread_create(worker_thread, &stack_space[THREAD_STACK_SIZE], 0, 4) == -1) {
		FAIL("Could not create thread\n");
	}
	h2_sem_down(&sema);
	for (i = 0; i < 100000; i++) {
		asm volatile (" nop ");
	}
	h2_sem_up(&semb);
	h2_sem_down(&sema);
	puts("TEST PASSED");
	exit(0);
}

#define MAX_SIZE (1024*1024)
unsigned char storage[MAX_SIZE] __attribute__((aligned(32)));
void spawn_vm(void *pc)
{
	unsigned int size;
	void *vmb;
	size = h2_config_vmblock_size(NUM_TOTAL_THREADS,1);
	printf("vmblock size: %d\n",size);
	if (size > MAX_SIZE) FAIL("Too much context needed\n");
	vmb = h2_config_vmblock_init(storage,SET_STORAGE_IDENT,0,0);
	printf("vmb: %p\n",vmb);
	vmb = h2_config_vmblock_init(vmb,SET_PMAP_TYPE,0,0);
	h2_config_vmblock_init(vmb,SET_CPUS_INTS,NUM_TOTAL_THREADS,1);
	h2_config_vmblock_init(vmb, SET_PRIO_TRAPMASK, 0x0, 0xffffffff);
	printf("initted\n");
	h2_vmboot(pc,&main_thread_stack[THREAD_STACK_SIZE-1],0,0,vmb);
	printf("vm booted\n");
}

int main() 
{
	int i;
	u32_t asid;
	H2K_mem_tlbfmt_t trans;
	h2_init(0);

	asm volatile (
	" %0 = ssr \n"
	" %0 = extractu(%0,#7,#8)\n" 
	: "=r"(asid));

	u32_t tlb_index = H2K_mem_tlb_probe(H2K_LINK_ADDR, asid);
	if (tlb_index == 0x80000000) {
		FAIL("Can't find monitor TLB entry");
	}
	u64_t tlb_entry = H2K_mem_tlb_read(tlb_index);
#if __QDSP6_ARCH__ <= 3
	tlb_entry |= 0x7ULL << 29;
#else
	tlb_entry |= 0xfULL << 28;
#endif
	H2K_mem_tlb_write(tlb_index, tlb_entry);

	/* Note that I start out in monitor mode, so just go screw with the TLB directly */
	/* Generate all permissions */
	for (i = 0; i < 16; i++) {
		trans = make_entry(0x90000000 + (i << 24),0x0,6,i,asid);
		H2K_mem_tlb_write(32+i,trans.raw);
#if ARCHV <= 3
		if (i & 1) {
			trans = make_entry(0x90000000 + (i << 24),0x0,6,i & -2,asid);
			H2K_mem_tlb_write(48+i,trans.raw);
		}
#endif
	}
	spawn_vm(vmmain);
	h2_thread_stop();
}

