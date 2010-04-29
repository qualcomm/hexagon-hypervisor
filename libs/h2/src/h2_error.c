/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <h2.h>

static void h2_default_error_handler()
{
	puts("Fatal error");
	h2_thread_stop();
}

static void (*errfuncs[16])(void) = {
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

void h2_handle_event0() { errfuncs[0x0](); }
void h2_handle_event1() { errfuncs[0x1](); }
void h2_handle_event2() { errfuncs[0x2](); }
void h2_handle_event3() { errfuncs[0x3](); }
void h2_handle_event4() { errfuncs[0x4](); }
void h2_handle_event5() { errfuncs[0x5](); }
void h2_handle_event6() { errfuncs[0x6](); }
void h2_handle_event7() { errfuncs[0x7](); }
void h2_handle_event8() { errfuncs[0x8](); }
void h2_handle_event9() { errfuncs[0x9](); }
void h2_handle_eventa() { errfuncs[0xa](); }
void h2_handle_eventb() { errfuncs[0xb](); }
void h2_handle_eventc() { errfuncs[0xc](); }
void h2_handle_eventd() { errfuncs[0xd](); }
void h2_handle_evente() { errfuncs[0xe](); }
void h2_handle_eventf() { errfuncs[0xf](); }

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

void h2_set_handler(int eventnum, void (*fn)())
{
	errfuncs[eventnum] = fn;
}

