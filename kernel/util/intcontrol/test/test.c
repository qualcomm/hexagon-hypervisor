/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <intcontrol.h>
#include <hexagon_protos.h>
#include <hw.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

void H2K_intcontrol_enable_TB(u32_t intno)
{
	return H2K_intcontrol_enable(intno);
}

void H2K_intcontrol_disable_TB(u32_t intno)
{
	return H2K_intcontrol_disable(intno);
}

void H2K_intcontrol_raise_TB(u32_t intno)
{
	return H2K_intcontrol_raise(intno);
}

u32_t fake_intctrl[0x400];

int main() 
{
	int i;
#if ARCHV >= 4
	int l2i;
#endif
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
#if ARCHV >= 4
	H2K_gp->l2_int_base = fake_intctrl;
	H2K_gp->l2_ack_base = fake_intctrl+0x200/sizeof(u32_t);
#endif
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		if ((ARCHV >= 4) && (i == L2_CORE_INTERRUPT)) continue;
		if (i < L1_INTERRUPTS) {
			H2K_intcontrol_disable_TB(i);
#if ARCHV > 3
			if (0 == (1 & (H2K_get_iad() >> i))) FAIL("Didn't disable L1");
#endif
			H2K_intcontrol_enable_TB(i);
			if (0 != (1 & (H2K_get_iad() >> i))) FAIL("Didn't enable L1");
		} else if (i < L2_INTERRUPT_START) {
			continue;
		} else {
#if ARCHV > 3
			l2i = i - L2_INTERRUPT_START;
			H2K_intcontrol_disable_TB(i);
			if (0 == (1 & (H2K_gp->l2_int_base[0x60+l2i/32] >> (i % 32)))) FAIL("didn't disable");
			H2K_intcontrol_enable_TB(i);
			if (0 == (1 & (H2K_gp->l2_int_base[0x80+l2i/32] >> (i % 32)))) FAIL("didn't enable");
#else
			FAIL("Should only be 32 interrupts on V3 and prior");
#endif
		}
	}

	/* DISABLE INTERRUPTS */
	asm volatile (
	" %0 = syscfg\n"
	" %0 = clrbit(%0,#4)\n"
	" syscfg = %0\n"
	" isync\n" : "=r"(i) : :"memory");

	for (i = 0; i < MAX_INTERRUPTS; i++) {
		if ((ARCHV >= 4) && (i == L2_CORE_INTERRUPT)) continue;
		if (i < L1_INTERRUPTS) {
			H2K_intcontrol_raise_TB(i);
			if (0 == (1 & (H2K_get_ipend() >> i))) FAIL("Didn't post interrupt");
		} else if (i < L2_INTERRUPT_START) {
			continue;
		} else {
#if ARCHV > 3
			l2i = i - L2_INTERRUPT_START;
			H2K_intcontrol_raise_TB(i);
			if (0 == (1 & (H2K_gp->l2_int_base[0x120 + l2i / 32] >> (i % 32)))) FAIL("Didn't raise");
#else
			FAIL("Should only be 32 interrupts on v3 and prior");
#endif
		}
	}

	puts("TEST PASSED\n");
	return 0;
}

