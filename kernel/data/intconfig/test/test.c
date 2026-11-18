/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <asm_offsets.h>
#include <context.h>
#include <stdlib.h>
#include <stdio.h>
#include <intconfig.h>
#include <setjmp.h>
#include <max.h>
#include <resched.h>
#include <globals.h>
#include <vmipi.h>
#include <timer.h>

#define BAD ((void *)(0xdeadbeef))

u32_t  TH_l2int_cfg[1024] __attribute__((aligned(4096))) ;

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

int main()
{
	int i;

	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

	H2K_gp->l2_int_base = TH_l2int_cfg;
	H2K_gp->l2_ack_base = TH_l2int_cfg + 0x80;

	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_gp->inthandlers[i].handler = BAD;
		H2K_gp->inthandlers[i].param = BAD;
	}
#if ARCHV >= 5
	H2K_intconfig_init(QDSP6SS_PRIV_BASE_LPASS);
#else
	H2K_intconfig_init(0);
#endif
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		if (i == RESCHED_INT) {
			if (H2K_gp->inthandlers[i].handler != H2K_resched)
				FAIL("wrong resched handler");
		} 
#if CLUSTER_SCHED
		else if (i == CLUSTER_RESCHED_INT) {
			if (H2K_gp->inthandlers[i].handler != H2K_resched_cluster)
				FAIL("wrong cluster resched handler");
		}
#endif
		 else if (i == VM_IPI_INT) {
			if (H2K_gp->inthandlers[i].handler != H2K_vm_ipi_do)
				FAIL("wrong ipi handler");
		} else if (i == TIMER_INT_CORE) {
			if (H2K_gp->inthandlers[i].handler != H2K_timer_int)
				FAIL("wrong timer int handler");
		} else {
			if (H2K_gp->inthandlers[i].handler != NULL) {
				FAIL("uninitialized handler");
			}
			if (H2K_gp->inthandlers[i].param != NULL) {
				FAIL("uninitialized param");
			}
		}
	}
	puts("TEST PASSED\n");
	return 0;
}
