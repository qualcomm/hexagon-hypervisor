/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <context.h>
#include <max.h>
#include <globals.h>
#include <linear.h>
#include <tlbfmt.h>
#include <asid.h>
#include <ctype.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

#define ENTRIES_MAX 32
H2K_linear_fmt_t lin[ENTRIES_MAX];

static inline H2K_linear_fmt_t gen_entry(u32_t pn)
{
	H2K_linear_fmt_t ret;
	ret.raw = 0;
	ret.xwru = 0xf;
	ret.ppn = ret.vpn = pn;
	return ret;
}

static inline H2K_linear_fmt_t gen_chain(void *p)
{
	H2K_linear_fmt_t ret;
	ret.raw = 0;
	ret.chain = 0x1;
	ret.low = (u32_t)p;
	return ret;
}

void make_list(const char *fmt)
{
	u32_t i;
	H2K_linear_fmt_t tmp;
	lin[ENTRIES_MAX-1].raw = 0;
	for (i = 0; i < ENTRIES_MAX-1; i++) {
		tmp.raw = 0;
		switch (fmt[i]) {
			case '0': lin[i] = tmp; continue;
			case '\0': lin[i] = tmp; break;
			case 'a': lin[i] = gen_entry(0xa0000); continue;
			case 'b': lin[i] = gen_entry(0xb0000); continue;
			case 'c': lin[i] = gen_entry(0xc0000); continue;
			case 'd': lin[i] = gen_entry(0xd0000); continue;
			case 'e': lin[i] = gen_entry(0xe0000); continue;
			case 'f': lin[i] = gen_entry(0xf0000); continue;
			case '!': lin[i] = gen_chain(&lin[i+2]); continue;
			default:
				FAIL("Unknown character in string");
		}
		if (fmt[i] == '\0') break;
	}
	a.ssr_asid = 0;
	H2K_mem_asid_table[0].ptb = (u32_t)(lin);
	H2K_mem_asid_table[0].count = 1;
}

void check_good(const char *good)
{
	int i;
	H2K_mem_tlbfmt_t tmp,check;
	u32_t badva;
	for (i = 0; good[i] != '\0'; i++) {
		badva = (0xa + good[i] - 'a') << 28;
		check.raw = 0;
#if __QDSP6_ARCH__ <= 3
		check.xwr = 0x7;
		check.valid = 1;
		check.ppn = check.vpn = (badva >> 12);
#else
		check.valid = 1;
		check.xwru = 0xe;
		check.ppd = badva >> 11 | 1;
		check.vpn = (badva >> 12);
#endif
		tmp = H2K_mem_translate_linear(badva,&a);
		if (tmp.raw != check.raw) {
			printf("%s(%c)\n",good,good[i]);
			FAIL("good string failed");
		}
	}
}

void check_bad(const char *bad)
{
	int i;
	u32_t badva;
	for (i = 0; bad[i] != '\0'; i++) {
		badva = (0xa + bad[i] - 'a') << 28;
		if (H2K_mem_translate_linear(badva,&a).raw != 0) {
			printf("%s(%c)\n",bad,bad[i]);
			FAIL("Bad string failed");
		}
	}
}

int main()
{
	__asm__ __volatile(" r16 = %0 " : : "r"(&H2K_kg));
	make_list(""); check_good(""); check_bad("abcdef");
	make_list("a0b"); check_good("a"); check_bad("bcdef");
	make_list("a!0b"); check_good("ab"); check_bad("cdef");
	make_list("a!0bc"); check_good("abc"); check_bad("def");
	make_list("a!00bc"); check_good("a"); check_bad("bcdef");
	make_list("a!bc"); check_good("ac"); check_bad("bdef");
	make_list("a!0!0bc"); check_good("abc"); check_bad("def");
	make_list("af!0!00bc"); check_good("af"); check_bad("bcde");
	make_list("af!0!00bc"); H2K_mem_asid_table[0].ptb += 8; check_good("f"); check_bad("abcde");
	puts("TEST PASSED");
	return 0;
}

