/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * bestwait_sim_check
 * ==================
 *
 * How it works:
 *   - BESTWAIT holds the priority of the best task waiting to run.
 *   - When the effective priority of ANY hardware thread (STID.PRIO) is worse
 *     than BESTWAIT, the hardware raises the reschedule interrupt (the interrupt
 *     number programmed into SCHEDCFG.INTNO) and resets BESTWAIT to 0x1FF.
 *   - IPEND reflects pending interrupts; "the hardware automatically sets bits
 *     in this register when an interrupt is received or delivered."
 *
 * This test arranges the exact trigger condition and checks:
 *   PART A: the BESTWAIT and SCHEDCFG registers can be read/written (storage).
 *   PART B: negative case - it does NOT fire when no thread is worse.
 *   PART C: equal priorities - it does NOT fire when STID.PRIO equals BESTWAIT.
 *   PART D: it does NOT fire when SCHEDCFG.EN=0 (the feature must be enabled,
 *           which the boot path H2K_init_setup normally does).
 *   PART E: hardware steering delivers RESCHED to a qualified thread whose
 *           RESCHED imask bit is clear (the boot/hw-steering model). This verifies
 *           that BESTWAIT fires again and is not stale by re-arming it and checking
 *           that the interrupt is posted again with IAD=0 and IPEND raising the
 *           RESCHED_INT_MASK bit.
 *
 * Interrupts are kept globally disabled (clear_gie) so that, if the interrupt
 * is posted, it stays pending in IPEND for us to observe rather than being taken.
 */

#include <c_std.h>
#include <stdio.h>
#include <hexagon_protos.h>
#include <hw.h>
#include <max.h>

static int failures;

static void check(const char *what, int ok)
{
	printf("  [%s] %s\n", ok ? "PASS" : "FAIL", what);
	if (!ok)
		failures++;
}

int main()
{
	u32_t bw, sc, ipend, iad, best_ready;
	u32_t hthread = get_hwtnum();

	/* Keep interrupts globally disabled so a posted RESCHED_INT remains pending
	 * in IPEND (taken interrupts auto-clear their IPEND bit; see spec 6.1). */
	H2K_clear_gie();
	H2K_clear_ipend(0xffffffff);

	puts("PART A: BESTWAIT / SCHEDCFG behave as readable/writable registers");
	u32_t garbage_value = 0x123;
	H2K_set_bestwait(garbage_value);
	bw = H2K_get_bestwait();
	iad = H2K_get_iad();
	ipend = H2K_get_ipend();

	printf("    BESTWAIT: wrote 0x%x, read 0x%x\n", garbage_value, bw);
	printf("    IPEND=0x%x (RESCHED bit %d), IAD=0x%x, BESTWAIT now 0x%x\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0, iad, bw);
	check("BESTWAIT round-trips", bw == garbage_value);
	check("IAD is 0", iad == 0);

	H2K_set_bestwait(BESTWAIT_MASK);

	sc = SCHEDCFG_EN | SCHEDCFG_INTNO(RESCHED_INT);
	H2K_set_schedcfg(sc);
	printf("    SCHEDCFG: wrote 0x%x, read 0x%x\n", sc, H2K_get_schedcfg());
	check("SCHEDCFG round-trips", H2K_get_schedcfg() == sc);

	puts("PART B: negative case - comparator must NOT fire when no thread is "
	     "worse than BESTWAIT");

	/* Make this thread a GOOD priority (10) and arm BESTWAIT WORSE (50).
	 * 10 (us) is better than 50 (bestwait) -> nothing is worse -> no fire. */
	H2K_clear_ipend(0xffffffff);
	set_thread_stid_prio(hthread, 10);
	best_ready = 50;
	H2K_set_bestwait(best_ready);
	ipend = H2K_get_ipend();
	iad = H2K_get_iad();
	bw = H2K_get_bestwait();
	printf("    state: STID.PRIO=10, BESTWAIT=0x%x\n", best_ready);
	printf("    IPEND=0x%x (RESCHED bit %d), IAD=0x%x, BESTWAIT now 0x%x\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0, iad, bw);
	check("comparator did NOT post RESCHED_INT (no thread worse)",
	      (ipend & RESCHED_INT_INTMASK) == 0);
	check("IAD is 0", iad == 0);
	check("BESTWAIT retained its value (not auto-reset)", bw == best_ready);

	puts("PART C: equal priorities - comparator must NOT fire when STID.PRIO equals BESTWAIT");

	/* Make this thread priority equal to BESTWAIT (50).
	 * 50 (us) is NOT worse than 50 (bestwait) -> no fire. */
	H2K_clear_ipend(0xffffffff);
	set_thread_stid_prio(hthread, 50);
	best_ready = 50;
	H2K_set_bestwait(best_ready);
	ipend = H2K_get_ipend();
	iad = H2K_get_iad();
	bw = H2K_get_bestwait();
	printf("    state: STID.PRIO=50, BESTWAIT=0x%x (equal priorities)\n", best_ready);
	printf("    IPEND=0x%x (RESCHED bit %d), IAD=0x%x, BESTWAIT now 0x%x\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0, iad, bw);
	check("comparator did NOT fire when priorities are equal",
	      (ipend & RESCHED_INT_INTMASK) == 0);
	check("IAD is 0", iad == 0);
	check("BESTWAIT retained its value when equal", bw == best_ready);

	puts("PART D: does the feature require SCHEDCFG.EN? (mimics the scenario "
	     "unit test, which never calls H2K_init_setup and so never enables it)");

	/* Clear only the EN bit (keep INTNO) to confirm EN specifically gates
	 * the feature -- not just any SCHEDCFG write. */
	sc = ~SCHEDCFG_EN | SCHEDCFG_INTNO(RESCHED_INT);
	H2K_set_schedcfg(sc);
	printf("    SCHEDCFG: wrote 0x%x, read 0x%x\n", sc, H2K_get_schedcfg());
	asm volatile ("isync");
	H2K_clear_ipend(0xffffffff);
	set_thread_stid_prio(hthread, 200);
	best_ready = 50;
	H2K_set_bestwait(best_ready);
	ipend = H2K_get_ipend();
	iad = H2K_get_iad();
	bw = H2K_get_bestwait();
	printf("    SCHEDCFG=~EN|INTNO (EN=0, INTNO kept), STID.PRIO=200, BESTWAIT=50\n");
	printf("    IPEND=0x%x (RESCHED bit %d), IAD=0x%x, BESTWAIT now 0x%x\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0, iad, bw);
	printf("    OBSERVATION: with EN=0, comparator fires=%d "
	       "(if 0, SCHEDCFG.EN is REQUIRED)\n",
	       (ipend & RESCHED_INT_INTMASK) ? 1 : 0);
	check("comparator does NOT fire when SCHEDCFG.EN=0",
	      (ipend & RESCHED_INT_INTMASK) == 0);
	check("IAD is 0", iad == 0);
	check("Bestwait retains its value (not auto-reset) when EN=0", bw == best_ready);

	puts("PART E: hardware steering delivers RESCHED to a qualified thread whose "
	     "RESCHED imask bit is clear (the boot/hw-steering model)");

	/* Model the hw-steering setup: feature enabled, this thread's RESCHED imask
	 * bit cleared via iassignw (every thread qualified), STID.PRIO made worse
	 * than BESTWAIT.  The hardware should steer RESCHED_INT to this thread and
	 * post it in IPEND.  Re-enable interrupts briefly is NOT done here (we keep
	 * GIE off so the posted bit stays observable in IPEND). */
	sc = SCHEDCFG_EN | SCHEDCFG_INTNO(RESCHED_INT);
	H2K_set_schedcfg(sc);
	printf("    SCHEDCFG: wrote 0x%x, read 0x%x\n", sc, H2K_get_schedcfg());
	asm volatile ("isync");
	iassignw(RESCHED_INT, 0);   /* clear RESCHED imask bit on all threads */
	H2K_clear_ipend(0xffffffff);
	set_thread_stid_prio(hthread, 200);
	best_ready = 50;
	H2K_set_bestwait(best_ready);
	ipend = H2K_get_ipend();
	iad = H2K_get_iad();
	bw = H2K_get_bestwait();
	printf("    IPEND=0x%x (RESCHED bit %d), IAD=0x%x, BESTWAIT now 0x%x\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0, iad, bw);
	check("RESCHED steered to this qualified thread",
	      (ipend & RESCHED_INT_INTMASK) != 0);
	check("IAD is 0", iad == 0);
	check("Bestwait retains its value (not auto-reset)", bw == BESTWAIT_MASK);

	/* Re-arm BESTWAIT to verify it fires again (not stale) */
	H2K_clear_ipend(0xffffffff);
	set_thread_stid_prio(hthread, 200);
	best_ready = 50;
	H2K_set_bestwait(best_ready);
	ipend = H2K_get_ipend();
	iad = H2K_get_iad();
	bw = H2K_get_bestwait();
	printf("    IPEND=0x%x (RESCHED bit %d), IAD=0x%x, BESTWAIT now 0x%x\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0, iad, bw);
	check("BESTWAIT fires again (not stale)", (ipend & RESCHED_INT_INTMASK) != 0);
	check("IAD is 0", iad == 0);


		   
	if (failures == 0) {
		puts("TEST PASSED");
		return 0;
	}

	/* If PART B fails, the demonstration is that the sim is storage-only. */
	puts("TEST FAILED");
	return 1;
}
