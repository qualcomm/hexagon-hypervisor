/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <intcontrol.h>

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

u32_t fake_intctrl[0x100];

static inline u32_t read_iad()
{
	u32_t ret;
	asm volatile ("%0 = iad" : "=r"(ret));
	return ret;
}

int main() 
{
	int i,l2i;
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	H2K_gp->l2_int_base = fake_intctrl;
	H2K_gp->l2_ack_base = fake_intctrl+0x200/sizeof(u32_t);
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		if ((ARCHV >= 4) && (i == 31)) continue;
		if (i < 32) {
			H2K_intcontrol_disable_TB(i);
#if ARCHV > 3
			if (0 == (1 & (read_iad() >> i))) FAIL("Didn't disable L1");
#endif
			H2K_intcontrol_enable_TB(i);
			if (0 != (1 & (read_iad() >> i))) FAIL("Didn't enable L1");
		} else {
			l2i = i-32;
			H2K_intcontrol_disable_TB(i);
			if (0 == (1 & (H2K_gp->l2_int_base[0x60+l2i/32] >> (i % 32)))) FAIL("didn't disable");
			H2K_intcontrol_enable_TB(i);
			if (0 == (1 & (H2K_gp->l2_int_base[0x80+l2i/32] >> (i % 32)))) FAIL("didn't enable");
		}
	}

	puts("TEST PASSED\n");
	return 0;
}

