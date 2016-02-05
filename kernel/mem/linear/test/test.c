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
#include <translate.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

H2K_asid_entry_t info = {
	.ptb = 0,
	.fields = {
		.count = 1,
		.vmid = 2,
		.type = H2K_ASID_TRANS_TYPE_LINEAR,
		.log_maxhops = 2,
		.extra = 0,
	},
};

H2K_vmblock_t myvmblock;

#define ENTRIES_MAX 32
H2K_linear_fmt_t lin[ENTRIES_MAX];

static inline H2K_linear_fmt_t gen_entry(u32_t pn)
{
	H2K_linear_fmt_t ret;
	ret.raw = 0;
	ret.xwru = 0xf;
	ret.cccc = 0x7;
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
	info.ptb = (u32_t)(lin);
}

void check_good(const char *good)
{
	int i;
	u32_t badva;
	H2K_translation_t trans;
	H2K_translation_t check;
	for (i = 0; good[i] != '\0'; i++) {
		badva = (0xa + good[i] - 'a') << 28;
		check.raw = 0;
		check.size = 0;
		check.xwru = 0xf;
		check.cccc = 0x7;
		check.pn = badva >> 12;
		trans = H2K_translate_default(badva);
		trans = H2K_linear_translate(trans,info);
		if (trans.raw != check.raw) {
			printf("%s(%c)\n",good,good[i]);
			printf("%016llx vs %016llx\n",trans.raw,check.raw);
			FAIL("good string failed");
		}
	}
}

void check_bad(const char *bad)
{
	int i;
	u32_t badva;
	H2K_translation_t trans;
	for (i = 0; bad[i] != '\0'; i++) {
		badva = (0xa + bad[i] - 'a') << 28;
		trans = H2K_translate_default(badva);
		if ((trans=H2K_linear_translate(trans,info)).raw != 0) {
			printf("%s(%c): %016llx\n",bad,bad[i],trans.raw);
			FAIL("Bad string failed");
		}
	}
}

int main()
{
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	H2K_gp->vmblocks[2] = &myvmblock;
	myvmblock.guestmap.raw = 0;
	make_list(""); check_good(""); check_bad("abcdef");
	make_list("a0b"); check_good("a"); check_bad("bcdef");
	make_list("a!0b"); check_good("ab"); check_bad("cdef");
	make_list("a!0bc"); check_good("abc"); check_bad("def");
	make_list("a!00bc"); check_good("a"); check_bad("bcdef");
	make_list("a!bc"); check_good("ac"); check_bad("bdef");
	make_list("a!0!0bc"); check_good("abc"); check_bad("def");
	make_list("af!0!00bc"); check_good("af"); check_bad("bcde");
	make_list("af!0!00bc"); info.ptb += 8; check_good("f"); check_bad("abcde");
	puts("TEST PASSED");
	return 0;
}

