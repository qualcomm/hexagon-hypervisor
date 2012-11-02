/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <h2.h>

static void h2_default_error_handler(int evt)
{
	unsigned int regs[4];
	unsigned int *ptr;
	unsigned int i;
	regs[0] = 0xcafe;
	regs[1] = 0xbabe;
	regs[2] = 0xdead;
	regs[3] = 0xbeef;
	printf("Fatal error: %d/0x%x",evt,evt);
	h2_vmtrap_getregs(regs);
	printf("g0: 0x%08x g1: 0x%08x g2: 0x%08x g3: 0x%08x\n",
		regs[0],regs[1],regs[2],regs[3]);
	ptr = (void *)h2_thread_myid();
	printf("ThreadID: %08x\n",(unsigned int)ptr);
	for (i = 0; i < 64; i+=4) {
		printf("%d: 0x%08x 0x%08x 0x%08x 0x%08x\n",i*4,ptr[i],ptr[i+1],ptr[i+2],ptr[i+3]);
	}
	asm ( " %0 = r30 " : "=r"(ptr));
	for (i = 0; i < 64; i+=4) {
	    printf("%08x: 0x%08x 0x%08x 0x%08x 0x%08x\n",(unsigned int)ptr+i,
		   ptr[i],ptr[i+1],ptr[i+2],ptr[i+3]);
	}
	h2_thread_stop(0);
}

static void (*errfuncs[16])(int) = {
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
	h2_default_error_handler,
};

void h2_handle_event0() { errfuncs[0x0](0x0); }
void h2_handle_event1() { errfuncs[0x1](0x1); }
void h2_handle_event2() { errfuncs[0x2](0x2); }
void h2_handle_event3() { errfuncs[0x3](0x3); }
void h2_handle_event4() { errfuncs[0x4](0x4); }
void h2_handle_event5() { errfuncs[0x5](0x5); }
void h2_handle_event6() { errfuncs[0x6](0x6); }
void h2_handle_event7() { errfuncs[0x7](0x7); }
void h2_handle_event8() { errfuncs[0x8](0x8); }
void h2_handle_event9() { errfuncs[0x9](0x9); }
void h2_handle_eventa() { errfuncs[0xa](0xa); }
void h2_handle_eventb() { errfuncs[0xb](0xb); }
void h2_handle_eventc() { errfuncs[0xc](0xc); }
void h2_handle_eventd() { errfuncs[0xd](0xd); }
void h2_handle_evente() { errfuncs[0xe](0xe); }
void h2_handle_eventf() { errfuncs[0xf](0xf); }

void h2_handle_errors()
{
	void *errtab;
	__asm__ __volatile__ (
	" %0 = r31\n"
	" call 1f \n"
	"2:\n"
	" jump h2_handle_event0\n"
	" jump h2_handle_event1\n"
	" jump h2_handle_event2\n"
	" jump h2_handle_event3\n"
	" jump h2_handle_event4\n"
	" jump h2_handle_event5\n"
	" jump h2_handle_event6\n"
	" jump h2_handle_event7\n"
	" jump h2_handle_event8\n"
	" jump h2_handle_event9\n"
	" jump h2_handle_eventa\n"
	" jump h2_handle_eventb\n"
	" jump h2_handle_eventc\n"
	" jump h2_handle_eventd\n"
	" jump h2_handle_evente\n"
	" jump h2_handle_eventf\n"
	"1:\n"
	" { %0 = r31; r31 = %0 }\n"
	: "=r"(errtab));
	h2_vmtrap_setvec(errtab);
}

void h2_set_handler(int eventnum, void (*fn)(int))
{
	errfuncs[eventnum] = fn;
}

