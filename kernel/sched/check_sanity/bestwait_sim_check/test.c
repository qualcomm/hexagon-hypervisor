/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * bestwait_sim_check
 * ==================
 * Demonstrates that the simulator (hexagon-sim + devsim_v81.cfg) does NOT model
 * the BESTWAIT/SCHEDCFG hardware reschedule-interrupt comparator described in
 * the V85 spec (80-V9418-400) section 6.5.
 *
 * Per the spec:
 *   - BESTWAIT holds the priority of the best task waiting to run.
 *   - When the effective priority of ANY hardware thread (STID.PRIO) is worse
 *     than BESTWAIT, the hardware raises the reschedule interrupt (the interrupt
 *     number programmed into SCHEDCFG.INTNO) and resets BESTWAIT to 0x1FF.
 *   - IPEND reflects pending interrupts; "the hardware automatically sets bits
 *     in this register when an interrupt is received or delivered."
 *
 * This test arranges the exact trigger condition and checks:
 *   PART A: the BESTWAIT and SCHEDCFG registers can be read/written (storage).
 *   PART B: with the feature enabled and this thread's STID.PRIO made WORSE than
 *           BESTWAIT, the hardware raises RESCHED_INT and resets BESTWAIT.
 *   PART C: negative case - it does NOT fire when no thread is worse.
 *   PART D: it does NOT fire when SCHEDCFG.EN=0 (the feature must be enabled,
 *           which the boot path H2K_init_setup normally does).
 *
 * Result observed on the current sim: ALL PASS - the comparator IS modeled.
 * (An earlier conclusion that it was not modeled was a measurement error: with
 * interrupts enabled the posted RESCHED_INT is immediately taken and its IPEND
 * bit auto-clears, so it reads back as 0.  This test disables interrupts first.)
 *
 * Interrupts are kept globally disabled (clear_gie) so that, if the interrupt
 * is posted, it stays pending in IPEND for us to observe rather than being
 * taken (which would auto-clear the IPEND bit per spec 6.1).
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <hexagon_protos.h>
#include <hw.h>
#include <max.h>

/* STID.PRIORITY is bits [23:16]; 0 = highest priority, 0xFF = lowest. */
#define STID_PRIO_SHIFT 16

static int failures;

static void check(const char *what, int ok)
{
	printf("  [%s] %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

int main()
{
	u32_t bw, sc, ipend;

	/* Keep interrupts globally disabled so a posted RESCHED_INT remains pending
	 * in IPEND (taken interrupts auto-clear their IPEND bit; see spec 6.1). */
	H2K_clear_gie();
	H2K_clear_ipend(0xffffffff);

	puts("PART A: BESTWAIT / SCHEDCFG behave as readable/writable registers");

	H2K_set_bestwait(0x123);
	asm volatile ("isync");
	bw = H2K_get_bestwait();
	printf("    BESTWAIT: wrote 0x123, read 0x%x\n", bw);
	check("BESTWAIT round-trips", bw == 0x123);

	sc = SCHEDCFG_EN | SCHEDCFG_INTNO(RESCHED_INT);
	H2K_set_schedcfg(sc);
	asm volatile ("isync");
	printf("    SCHEDCFG: wrote 0x%x, read 0x%x\n", sc, H2K_get_schedcfg());
	check("SCHEDCFG round-trips", H2K_get_schedcfg() == sc);

	puts("PART B: hardware comparator should raise RESCHED_INT when a thread's "
	     "STID.PRIO is worse than BESTWAIT");

	/* Make THIS thread look like the worst-priority running thread (prio 200). */
	asm volatile ("stid = %0; isync" : : "r"(200u << STID_PRIO_SHIFT));

	/* Enable the feature and arm BESTWAIT with a BETTER priority (50).
	 * 200 (us) is worse than 50 (bestwait) -> the spec says fire. */
	H2K_set_schedcfg(SCHEDCFG_EN | SCHEDCFG_INTNO(RESCHED_INT));
	asm volatile ("isync");
	H2K_set_bestwait(50);
	asm volatile ("isync");

	ipend = H2K_get_ipend();
	bw = H2K_get_bestwait();
	printf("    armed: STID.PRIO=200, BESTWAIT=50, SCHEDCFG.EN=1, INTNO=%d\n",
	       RESCHED_INT);
	printf("    IPEND=0x%x (RESCHED bit %d), BESTWAIT now 0x%x\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0, bw);

	check("hardware posted RESCHED_INT into IPEND",
	      (ipend & RESCHED_INT_INTMASK) != 0);
	check("hardware reset BESTWAIT to 0x1FF after firing", bw == 0x1ff);

	puts("PART C: negative case - comparator must NOT fire when no thread is "
	     "worse than BESTWAIT");

	/* Make this thread a GOOD priority (10) and arm BESTWAIT WORSE (50).
	 * 10 (us) is better than 50 (bestwait) -> nothing is worse -> no fire. */
	H2K_clear_ipend(0xffffffff);
	asm volatile ("stid = %0; isync" : : "r"(10u << STID_PRIO_SHIFT));
	H2K_set_bestwait(50);
	asm volatile ("isync");
	ipend = H2K_get_ipend();
	bw = H2K_get_bestwait();
	printf("    armed: STID.PRIO=10, BESTWAIT=50\n");
	printf("    IPEND=0x%x (RESCHED bit %d), BESTWAIT now 0x%x\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0, bw);
	check("comparator did NOT post RESCHED_INT (no thread worse)",
	      (ipend & RESCHED_INT_INTMASK) == 0);
	check("BESTWAIT retained its value (not auto-reset)", bw == 50);

	puts("PART D: does the feature require SCHEDCFG.EN? (mimics the scenario "
	     "unit test, which never calls H2K_init_setup and so never enables it)");

	/* Disable the feature entirely (EN=0), then set up the SAME firing
	 * condition as PART B: worst thread (200) vs BESTWAIT (50). */
	H2K_set_schedcfg(0);
	asm volatile ("isync");
	H2K_clear_ipend(0xffffffff);
	asm volatile ("stid = %0; isync" : : "r"(200u << STID_PRIO_SHIFT));
	H2K_set_bestwait(50);
	asm volatile ("isync");
	ipend = H2K_get_ipend();
	bw = H2K_get_bestwait();
	printf("    SCHEDCFG=0 (disabled), STID.PRIO=200, BESTWAIT=50\n");
	printf("    IPEND=0x%x (RESCHED bit %d), BESTWAIT now 0x%x\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0, bw);
	printf("    OBSERVATION: with EN=0, comparator fires=%d "
	       "(if 0, SCHEDCFG.EN is REQUIRED)\n",
	       (ipend & RESCHED_INT_INTMASK) ? 1 : 0);
	check("comparator does NOT fire when SCHEDCFG.EN=0",
	      (ipend & RESCHED_INT_INTMASK) == 0);

	if (failures == 0) {
		puts("TEST PASSED");
		puts("  => the simulator MODELS the BESTWAIT comparator: it raises "
		     "RESCHED_INT exactly when a thread's STID.PRIO is worse than "
		     "BESTWAIT and SCHEDCFG.EN=1, and not otherwise.");
		return 0;
	}

	/* If PART B fails, the demonstration is that the sim is storage-only. */
	puts("TEST FAILED");
	puts("  => registers are storage-only; the simulator does NOT model the");
	puts("     BESTWAIT hardware reschedule-interrupt comparator (spec 6.5).");
	puts("     The reschedule interrupt is therefore never raised by hardware");
	puts("     under simulation, even though the trigger condition is met.");
	return 1;
}
