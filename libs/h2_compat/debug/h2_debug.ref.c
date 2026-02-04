/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <stdio.h>
#include <h2.h>
#include <asm_offsets.h>

#define PRINT_R4(NAME0,NAME1,NAME2,NAME3) \
	printf("r" #NAME0 ":0x%08x r" #NAME1 ":0x%08x r" #NAME2 ":0x%08x r" #NAME3 ":0x%08x\n", context->r##NAME0, context->r##NAME1, context->r##NAME2, context->r##NAME3);

#define PRINT_R8(A,B,C,D,E,F,G,H) \
	printf("r" #A ": 0x%08x\tr" #B ": 0x%08x\tr" #C ": 0x%08x\tr" #D ": 0x%08x\tr" #E ": 0x%08x\tr" #F ": 0x%08x\tr" #G ": 0x%08x\tr" #H ": 0x%08x\n", \
		context->r##A, context->r##B, context->r##C, context->r##D, context->r##E, context->r##F, context->r##G, context->r##H);

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
	val_t val, val1;
	unsigned int dm[6];

	// Get monitor mode per-thread control registers
	unsigned int id = h2_thread_myid();

	h2_mutex_lock(&debug_mutex);
	val.raw = h2_thread_state(id, CONTEXT_PSEUDO_SGP0);
	printf("DEBUG DUMP\n");
	printf("tid:\t\t0x%08x\tcause:\t\t0x%02x\t\tpcycles:\t0x%016llx\n", id, context->g1 & 0xff, h2_get_core_pcycles());
	printf("E-PC(g0):\t0x%08x\tbadva(g3):\t0x%08x\tsgp0:\t\t0x%08x\n",
	       context->g0, context->g3, val.word0);

	/* Jam the cause in ssr value; ssr_cause has since been overwritten by trap calls  */
	val.raw = h2_thread_state(id, CONTEXT_ccrssr);
	val.byte0 = context->g1 & 0xff;
	unsigned int ccr = val.word1;
	unsigned int ssr = val.word0;

	val.raw = h2_thread_state(id, CONTEXT_status_prio_hthread_tid);
	printf("ccr:\t\t0x%08x\tssr:\t\t0x%08x\tprio:\t\t0x%02x\t\ththread: 0x%02x  tid: 0x%02x\n",
	       ccr, ssr, val.byte2, val.byte1, val.byte0);

	// get Exception Link Register (elr)
	val.raw = h2_thread_state(id, CONTEXT_elr);
	// get Guest Event Vector Base Address (gevb)
	val1.raw = h2_thread_state(id, CONTEXT_gevb);
	printf("elr:\t\t0x%08x\tgevb:\t\t0x%08x\n",
	       val.word0, val1.word0);
	
	val.raw = h2_thread_state(id, CONTEXT_PSEUDO_IMASK);
	printf("framekey:\t0x%08x\tframelimit:\t0x%08x\timask:\t0x%08x\n",
		context->framekey, context->framelimit, val.word0);

	#if ARCHV >= 73  // FIXME: Make this 79 if there is a separate build
	// get VTCM Window Access Register (vwctrl)
	val.raw = h2_thread_state(id, CONTEXT_vwctrl);
	printf("vwctrl:\t\t0x%08x\n\n", val.word0);
	#endif

	PRINT_R8(31,30,29,28,27,26,25,24);
	PRINT_R8(23,22,21,20,19,18,17,16);
	PRINT_R8(15,14,13,12,11,10,09,08);
	PRINT_R8(07,06,05,04,03,02,01,00);

	printf("\n");
	printf("g3:  0x%08x\tg2:  0x%08x\tg1:  0x%08x\tg0:  0x%08x\tcs1: 0x%08x\tcs0:  0x%08x\n", context->g3, context->g2, context->g1, context->g0, context->cs1, context->cs0);
	printf("gp:  0x%08x\tugp: 0x%08x\tm1:  0x%08x\tm0:  0x%08x\tusr: 0x%08x\tp3_0: 0x%08x\n", context->gp, context->ugp, context->m1, context->m0, context->usr, context->p3_0);
	printf("lc1: 0x%08x\tsa1: 0x%08x\tlc0: 0x%08x\tsa0: 0x%08x\n", context->lc1, context->sa1, context->lc0, context->sa0);

	printf("\nDMA: ");
	for (i = 0; i < 6; i++) {
		dm[i] = h2_hwconfig_dma_getcfg(i);
	}
	printf("DM0: 0x%08x\tDM1: 0x%08x\tDM2: 0x%08x\tDM3: 0x%08x\tDM4: 0x%08x\tDM5: 0x%08x\n", dm[0], dm[1], dm[2], dm[3], dm[4], dm[5]);

	printf("\nStack:\n");
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
