/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <vmdefs.h>
#include <context.h>
#include <readylist.h>
#include <hw.h>
#include <max.h>
#include <stdio.h>
#include <stdlib.h>
#include <globals.h>
#include <setjmp.h>
#include <atomic.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

s32_t ret;

jmp_buf env;
/* need this to prevent pulling in vmfuncs */ u32_t H2K_disable_guest_interrupts(H2K_thread_context *me) {
	return H2K_atomic_clrbit(&me->atomic_status_word,H2K_VMSTATUS_IE_BIT);
}

void H2K_fatal_thread(s16_t error_id, H2K_thread_context *me, u32_t info0, u32_t info1, u32_t hthread) 
{
	FAIL("fatal_thread called.");
}

void H2K_vmtrap_version(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0; }
void H2K_vmtrap_return(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 1; }
void H2K_vmtrap_setvec(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 2; }
void H2K_vmtrap_setie(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 3; }
void H2K_vmtrap_getie(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 4; }
void H2K_vmtrap_intop(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 5; }
void H2K_vmtrap_clrmap(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0xa; }
void H2K_vmtrap_newmap(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0xb; }
void H2K_vmtrap_cachectl(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0xd; }
void H2K_vmtrap_get_pcycles(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0xe; }
void H2K_vmtrap_set_pcycles(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0xf; }
void H2K_vmtrap_wait(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0x10; }
void H2K_vmtrap_yield(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0x11; }
void H2K_vmtrap_start(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0x12; }
void H2K_vmtrap_stop(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0x13; }
void H2K_vmtrap_vmpid(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0x14; }
void H2K_vmtrap_setregs(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0x15; }
void H2K_vmtrap_getregs(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0x16; }
void H2K_vmtrap_timer(H2K_thread_context *me) { if (me != &a) FAIL("bad ptr"); me->r00 = 0x18; }

s32_t call_trap1(u32_t trapnum, H2K_thread_context *context);
void TH_vectors();

void guest_mode();
void user_mode();

u64_t guest_stack[128] __attribute__((aligned(128*8)));

s32_t testvals[] = {
	 0, 1, 2, 3, 4, 5,-1,-1,-1,-1,10,11,-1,13,14,15,
	16,17,18,19,20,21,22,-1,24,-1,-1,-1,-1,-1,-1,-1
};

H2K_kg_t H2K_kg;

void TH_bad_vec()
{
	FAIL("Bad vector");
}

static inline void setup_guest()
{
	a.gevb = TH_vectors;
	a.gosp = (u32_t)(&guest_stack[127]);
//Write guest regs if in v4
#if ARCHV >= 4
	u32_t g2;
	u64_t g32;
	g2 = (u32_t)(&guest_stack[127]);
	g32 = 0;
	g32 |= g2;
	__asm__ __volatile( " g3:2 = %0\n" : : "r"(g32) );
#endif
}

u32_t TH_expected_guest_stack;
u32_t TH_saw_guest_error;

void TH_guest_trap()
{
	/* Check if stack is in expected location */
	u32_t is_guest_stack;
	is_guest_stack = (((((u32_t)(&is_guest_stack)) ^ ((u32_t)(&guest_stack[120]))) & (-(sizeof(guest_stack)))) == 0);
	if (is_guest_stack != TH_expected_guest_stack) {
	    printf("&is_guest_stack=0x%x, &guest_stack[120]=0x%x,mask=%x\n",(unsigned int)&is_guest_stack,(unsigned int)&guest_stack[120],-(sizeof(guest_stack)));
		printf("is_guest_stack=%d expected=%d\n",is_guest_stack,TH_expected_guest_stack);
		FAIL("Unexpected user/guest stack switch");
	}
	TH_saw_guest_error = 1;
	longjmp(env,1);
}

//void H2K_traptab();
//char H2K_stacks;

int main() 
{
	s32_t i;
	//	H2K_kg.traptab_addr = H2K_traptab;
	//	H2K_kg.stacks_addr = &H2K_stacks;

	for (i = 0; i < 32; i++) {
		setup_guest();
		user_mode();
		TH_expected_guest_stack = 1;
		TH_saw_guest_error = 0;
		if (setjmp(env) == 0) {
		    ret = call_trap1(i,&a);
		}
		if (TH_saw_guest_error == 0) {
			printf("test %d: ret=0x%x\n",i,ret);
			FAIL("Called vmtrap from user mode");
		}
	}
	setup_guest();
	guest_mode();
	TH_expected_guest_stack = 0;
	for (i = 0; i < 32; i++) {
		ret = -1;
		TH_saw_guest_error = 0;
		if (setjmp(env) == 0) ret = call_trap1(i,&a);
		if (testvals[i] == 0) {
			if (TH_saw_guest_error != 0) FAIL("Didn't error");
		} else {
			if (testvals[i] != ret) {
				printf("test %d: ret=0x%x, expected=0x%x\n",i,ret,testvals[i]);
				FAIL("Bad call/return");
			}
		}
	}

	puts("TEST PASSED\n");
	return 0;
}

