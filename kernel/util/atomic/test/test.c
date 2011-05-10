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

#define MASKBITS(N) ((1<<(N))-1)
#define INSERT(WORD,NEW,WIDTH,OFF) (((WORD) & (~((MASKBITS(WIDTH))<<(OFF)))) | (((NEW) & (MASKBITS(WIDTH))) << (OFF)))

u32_t word;

u32_t TH_atomic_setbit(u32_t *word, u32_t bit)
{
	return H2K_atomic_setbit(word,bit);
}

u32_t TH_atomic_clrbit(u32_t *word, u32_t bit)
{
	return H2K_atomic_clrbit(word,bit);
}

u32_t TH_atomic_swap(u32_t *word, u32_t val)
{
	return H2K_atomic_swap(word,val);
}

u32_t TH_atomic_insert(u32_t *word, u32_t val, u32_t width, u32_t offset)
{
	return H2K_atomic_insert(word,val,width,offset);
}

int main()
{
	word = 0;
	u32_t i,j;
	u32_t mask;
	u32_t tmp;
	u32_t last;

	/* For each bit in a word, check to make sure that setbit and clearbit work for OK case */
	for (i = 0; i < 32; i++) {
		mask = 0xFFFFFFFF>>(31-i);
		if (TH_atomic_setbit(&word,i) == 0) FAIL("First setbit failed");
		if ((word & (1<<i)) == 0) FAIL("Setbit didn't set bit");
		if (word != mask) FAIL("Unexpected word value (a)"); 
		if (TH_atomic_setbit(&word,i) != 0) FAIL("Second setbit succeeded");
		if ((word & (1<<i)) == 0) FAIL("Setbit cleared bit");
		if (word != mask) FAIL("Unexpected word value (b)");
	}
	/* For each bit in a word, check to make sure that setbit and clrbit work for the fail case */
	for (i = 0; i < 32; i++) {
		mask = 0xFFFFFFFF<<(i+1);
		if (TH_atomic_clrbit(&word,i) == 0) FAIL("First clrbit failed");
		if ((word & (1<<i)) != 0) FAIL("clrbit didn't set bit");
		if (word != mask) FAIL("Unexpected word value (c)"); 
		if (TH_atomic_clrbit(&word,i) != 0) FAIL("Second clrbit succeeded");
		if ((word & (1<<i)) != 0) FAIL("clrbit set bit");
		if (word != mask) FAIL("Unexpected word value (d)");
	}
	/* Make sure that swap swaps values */
	last = word = 0;
	for (i = 0; i < 32; i++) {
		tmp = rand();
		if (last != TH_atomic_swap(&word,tmp)) FAIL("Swap returned wrong val");
		if (word != tmp) FAIL("Swap didn't set val");
		last = tmp;
	}
	last = 0;
	/* Make sure that insert works */
	for (i = 0; i < 16; i++) {
		for (j = 15; j > 0; j--) {
			tmp = INSERT(word,0xcafe,i,j);
			if (tmp != TH_atomic_insert(&word,0xcafe,i,j)) FAIL("Insert incorrect (ret)");
			if (tmp != word) FAIL("Insert incorrect");
		}
	}
	puts("TEST PASSED");
	return 0;
}

