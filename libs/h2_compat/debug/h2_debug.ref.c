/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <h2.h>
#include <asm_offsets.h>

#define PRINT_RPAIR(NAME0,NAME1) \
	printf("r"#NAME0 ":\t0x%08x\tr" #NAME1 ":\t0x%08x\n", context->r##NAME0, context->r##NAME1);

#define PRINT_CPAIR(NAME0,NAME1) \
	printf(#NAME0 ":\t0x%08x\t" #NAME1 ":\t0x%08x\n", context->NAME0, context->NAME1);

static h2_mutex_t debug_mutex;

typedef union {
	struct {
		unsigned int word0;
		unsigned int word1;
	};
	struct {
		unsigned char byte0;
		unsigned char byte1;
		unsigned char byte2;
		unsigned char byte3;
	};
	unsigned long long int raw;
} val_t;

void h2_debug_context_dump(h2_context_t *context)
{
	unsigned int *ptr;
	unsigned int i;
	val_t val;

	unsigned int id = h2_thread_myid();
	val.raw = h2_thread_state(id, CONTEXT_ccrssr);

	h2_mutex_lock(&debug_mutex);
	printf("DEBUG DUMP threadid = 0x%08x, core_pcycles = 0x%016llx\n", id, h2_get_core_pcycles());
	printf("Exception PC (g0) = 0x%08x, cause (g1 & 0xff) = 0x%02x, badva (g3) = 0x%08x\n\n", context->g0, context->g1 & 0xff, context->g3);

	/* Jam the cause in ssr value; ssr_cause has since been overwritten by trap calls  */
	val.byte0 = context->g1 & 0xff;
	printf("ccr: 0x%08x\tssr: 0x%08x\n", val.word1, val.word0);

	val.raw = h2_thread_state(id, CONTEXT_status_prio_hthread_tid);
	printf("prio: 0x%02x\ththread: 0x%02x\ttid: 0x%02x\n\n", val.byte2, val.byte1, val.byte0);

	PRINT_RPAIR(31,30);
	PRINT_RPAIR(29,28);
	PRINT_RPAIR(27,26);
	PRINT_RPAIR(25,24);
	PRINT_RPAIR(23,22);
	PRINT_RPAIR(21,20);
	PRINT_RPAIR(19,18);
	PRINT_RPAIR(17,16);
	PRINT_RPAIR(15,14);
	PRINT_RPAIR(13,12);
	PRINT_RPAIR(11,10);
	PRINT_RPAIR(09,08);
	PRINT_RPAIR(07,06);
	PRINT_RPAIR(05,04);
	PRINT_RPAIR(03,02);
	PRINT_RPAIR(01,00);
	PRINT_CPAIR(g3,g2);
	PRINT_CPAIR(g1,g0);
	PRINT_CPAIR(cs1,cs0);
	PRINT_CPAIR(gp,ugp);
	PRINT_CPAIR(m1,m0);
	PRINT_CPAIR(usr,p3_0);
	PRINT_CPAIR(lc1,sa1);
	PRINT_CPAIR(lc0,sa0);
	printf("\n");

	printf("\nStack:\n\n");
	ptr = (unsigned int *)(context->r29);
	for (i = 0 ; i < 64 ; i += 4) {
		printf("%08x: 0x%08x 0x%08x 0x%08x 0x%08x\n",(unsigned int)(ptr + i),
		   ptr[i],ptr[i+1],ptr[i+2],ptr[i+3]);
	}
	printf("\n");

	h2_mutex_unlock(&debug_mutex);
}

static void (*h2_debug_context_handler)(h2_context_t *context) = h2_debug_context_dump;

void h2_debug_context_handle(h2_context_t *context)
{
	h2_debug_context_handler(context);
}

void h2_debug_register_handler(void (*handler)(h2_context_t *context))
{
	h2_debug_context_handler = handler;
}

void h2_debug_mutex_init() {
	h2_mutex_init(&debug_mutex);
}
