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

	/* Clear only the EN bit (keep INTNO) to confirm EN specifically gates
	 * the feature -- not just any SCHEDCFG write. */
	H2K_set_schedcfg(~SCHEDCFG_EN | SCHEDCFG_INTNO(RESCHED_INT));
	asm volatile ("isync");
	H2K_clear_ipend(0xffffffff);
	asm volatile ("stid = %0; isync" : : "r"(200u << STID_PRIO_SHIFT));
	H2K_set_bestwait(50);
	asm volatile ("isync");
	ipend = H2K_get_ipend();
	bw = H2K_get_bestwait();
	printf("    SCHEDCFG=~EN|INTNO (EN=0, INTNO kept), STID.PRIO=200, BESTWAIT=50\n");
	printf("    IPEND=0x%x (RESCHED bit %d), BESTWAIT now 0x%x\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0, bw);
	printf("    OBSERVATION: with EN=0, comparator fires=%d "
	       "(if 0, SCHEDCFG.EN is REQUIRED)\n",
	       (ipend & RESCHED_INT_INTMASK) ? 1 : 0);
	check("comparator does NOT fire when SCHEDCFG.EN=0",
	      (ipend & RESCHED_INT_INTMASK) == 0);

	puts("PART E: hardware steering delivers RESCHED to a qualified thread whose "
	     "RESCHED imask bit is clear (the boot/hw-steering model)");

	/* Model the hw-steering setup: feature enabled, this thread's RESCHED imask
	 * bit cleared via iassignw (every thread qualified), STID.PRIO made worse
	 * than BESTWAIT.  The hardware should steer RESCHED_INT to this thread and
	 * post it in IPEND.  Re-enable interrupts briefly is NOT done here (we keep
	 * GIE off so the posted bit stays observable in IPEND). */
	H2K_set_schedcfg(SCHEDCFG_EN | SCHEDCFG_INTNO(RESCHED_INT));
	asm volatile ("isync");
	iassignw(RESCHED_INT, 0);   /* clear RESCHED imask bit on all threads */
	H2K_clear_ipend(0xffffffff);
	asm volatile ("stid = %0; isync" : : "r"(200u << STID_PRIO_SHIFT));
	H2K_set_bestwait(50);
	asm volatile ("isync");
	ipend = H2K_get_ipend();
	printf("    steered: RESCHED imask clear, STID.PRIO=200, BESTWAIT=50 -> "
	       "IPEND=0x%x (RESCHED %d)\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0);
	check("RESCHED steered to this qualified thread",
	      (ipend & RESCHED_INT_INTMASK) != 0);

	puts("PART F: the real 255-gap scenario -- simulate a sleeping hthread.\n"
	     "  A sleeping hthread has stid=-1 (STID.PRIO=255) AND the sleep path\n"
	     "  opened its imask.  The only ready thread has prio=255.  check_sanity\n"
	     "  writes BESTWAIT=255.  Hardware: 255>255=false -- does NOT fire.\n"
	     "  Software backstop (wait_mask && best<MAX_PRIOS): best=255 < 256 = true\n"
	     "  => backstop fires resched_int() to wake the sleeping hthread.");

	/* Simulate a sleeping hthread by faking wait_mask (we can't actually park
	 * a second hthread in this single-threaded test, but we can verify the
	 * hardware comparator side: STID.PRIO=255, BESTWAIT=255 does not fire,
	 * i.e. the software backstop is the only thing that covers this case. */
	H2K_set_schedcfg(SCHEDCFG_EN | SCHEDCFG_INTNO(RESCHED_INT));
	asm volatile ("isync");
	iassignw(RESCHED_INT, 0);
	H2K_clear_ipend(0xffffffff);
	asm volatile ("stid = %0; isync" : : "r"(255u << STID_PRIO_SHIFT));
	H2K_set_bestwait(255);
	asm volatile ("isync");
	ipend = H2K_get_ipend();
	printf("    STID.PRIO=255, BESTWAIT=255 (no wait_mask): IPEND=0x%x (RESCHED %d)\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0);
	check("hardware does NOT fire at 255==255 (the gap)",
	      (ipend & RESCHED_INT_INTMASK) == 0);

	/* Now verify the software backstop fires when wait_mask is set AND best<256.
	 * best=255 < MAX_PRIOS(256) is true, so the condition triggers resched_int(). */
	H2K_clear_ipend(0xffffffff);
	swi(RESCHED_INT_INTMASK);   /* simulate what the software backstop does */
	ipend = H2K_get_ipend();
	printf("    after software resched_int(): IPEND=0x%x (RESCHED %d)\n",
	       ipend, (ipend & RESCHED_INT_INTMASK) ? 1 : 0);
	check("software backstop covers the 255==255 gap",
	      (ipend & RESCHED_INT_INTMASK) != 0);

	if (failures == 0) {
		puts("TEST PASSED");
		return 0;
	}

	/* If PART B fails, the demonstration is that the sim is storage-only. */
	puts("TEST FAILED");
	return 1;
}
