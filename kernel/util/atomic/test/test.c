/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <atomic.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

u32_t word;

u32_t TH_atomic_setbit(u32_t *word, u32_t bit)
{
	return H2K_atomic_setbit(word,bit);
}

u32_t TH_atomic_clrbit(u32_t *word, u32_t bit)
{
	return H2K_atomic_clrbit(word,bit);
}

int main()
{
	word = 0;
	u32_t i;
	u32_t mask;

	for (i = 0; i < 32; i++) {
		mask = 0xFFFFFFFF>>(31-i);
		if (TH_atomic_setbit(&word,i) == 0) FAIL("First setbit failed");
		if ((word & (1<<i)) == 0) FAIL("Setbit didn't set bit");
		if (word != mask) FAIL("Unexpected word value (a)"); 
		if (TH_atomic_setbit(&word,i) != 0) FAIL("Second setbit succeeded");
		if ((word & (1<<i)) == 0) FAIL("Setbit cleared bit");
		if (word != mask) FAIL("Unexpected word value (b)");
	}

	for (i = 0; i < 32; i++) {
		mask = 0xFFFFFFFF<<(i+1);
		if (TH_atomic_clrbit(&word,i) == 0) FAIL("First clrbit failed");
		if ((word & (1<<i)) != 0) FAIL("clrbit didn't set bit");
		if (word != mask) FAIL("Unexpected word value (c)"); 
		if (TH_atomic_clrbit(&word,i) != 0) FAIL("Second clrbit succeeded");
		if ((word & (1<<i)) != 0) FAIL("clrbit set bit");
		if (word != mask) FAIL("Unexpected word value (d)");
	}

	puts("TEST PASSED");
	return 0;
}

