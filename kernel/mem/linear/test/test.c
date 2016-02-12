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

H2K_asid_entry_t stage2_info = {
	.ptb = 0,
	.fields = {
		.count = 1,
		.vmid = 3,
		.type = H2K_ASID_TRANS_TYPE_OFFSET,
		.log_maxhops = 2,
		.extra = 0,
	},
};

H2K_vmblock_t myvmblock;
H2K_vmblock_t myvmblock2;

int TH_expected_translate_calls = 0;
int TH_translate_idx = 0;
#define MAX_TRANSLATE_CALLS 8
u32_t TH_expected_translate_pn[MAX_TRANSLATE_CALLS];
H2K_asid_entry_t TH_expected_translate_info;

u32_t TH_xwru_mask = ~0;

typedef H2K_translation_t (*translation_funcptr)(H2K_translation_t in, H2K_asid_entry_t info);
translation_funcptr TH_translate_handlers[MAX_TRANSLATE_CALLS];

H2K_translation_t TH_translate_passthrough(H2K_translation_t in, H2K_asid_entry_t info)
{
	// printf("translate: pass: in=%016llx info=%016llx\n",in.raw,info.raw);
	in.xwru &= TH_xwru_mask;
	return in;
}

H2K_translation_t TH_translate_fail(H2K_translation_t in, H2K_asid_entry_t info)
{
	// printf("translate: fail: in=%016llx info=%016llx\n",in.raw,info.raw);
	in.raw = 0;
	return in;
}

H2K_translation_t H2K_translate(H2K_translation_t in, H2K_asid_entry_t info)
{
	if (TH_expected_translate_calls == TH_translate_idx) FAIL("unexpected translate");
	if (info.raw != TH_expected_translate_info.raw) FAIL("translate info wrong");
	if (TH_expected_translate_pn[TH_translate_idx] != in.pn) FAIL("translate pn wrong");
	return TH_translate_handlers[TH_translate_idx++](in,info);
}

#define ENTRIES_MAX 2048
H2K_linear_fmt_t lin[ENTRIES_MAX] __attribute__((aligned(4096)));

#define X4_A "aaaa"
#define X16_A X4_A X4_A X4_A X4_A 
#define X64_A X16_A X16_A X16_A X16_A 
#define X256_A X64_A X64_A X64_A X64_A 
#define X512_A X256_A X256_A 

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
		check.xwru = 0xf & TH_xwru_mask;
		check.cccc = 0x7;
		check.pn = badva >> 12;
		TH_translate_idx = 0;
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
		TH_translate_idx = 0;
		trans = H2K_translate_default(badva);
		if ((trans=H2K_linear_translate(trans,info)).raw != 0) {
			printf("%s(%c): %016llx\n",bad,bad[i],trans.raw);
			FAIL("Bad string failed");
		}
	}
}

int main()
{
	int i;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	H2K_gp->vmblocks[2] = &myvmblock;
	H2K_gp->vmblocks[3] = &myvmblock2;
	myvmblock.guestmap.raw = 0;
	TH_expected_translate_calls = 0;
	make_list(""); check_good(""); check_bad("abcdef");
	make_list("a0b"); check_good("a"); check_bad("bcdef");
	make_list("a!0b"); check_good("ab"); check_bad("cdef");
	make_list("a!0bc"); check_good("abc"); check_bad("def");
	make_list("a!00bc"); check_good("a"); check_bad("bcdef");
	make_list("a!bc"); check_good("ac"); check_bad("bdef");
	make_list("a!0!0bc"); check_good("abc"); check_bad("def");
	make_list("af!0!00bc"); check_good("af"); check_bad("bcde");
	make_list("af!0!00bc"); info.ptb += 8; check_good("f"); check_bad("abcde");

	/* Check translations around walking */

	H2K_gp->vmblocks[2]->guestmap = stage2_info;
	TH_expected_translate_info = stage2_info;
	make_list(X512_A "ab");

	TH_expected_translate_calls = 2;
	TH_translate_handlers[0] = TH_translate_passthrough;
	TH_translate_handlers[1] = TH_translate_passthrough;
	TH_translate_handlers[2] = TH_translate_passthrough;
	TH_translate_handlers[3] = TH_translate_fail;
	TH_expected_translate_pn[0] = ((unsigned long)(&lin[0])) >> 12;
	TH_expected_translate_pn[1] = 0xa0000;
	check_good("a");

	TH_expected_translate_calls = 3;
	TH_expected_translate_pn[0] = ((unsigned long)(&lin[0])) >> 12;
	TH_expected_translate_pn[1] = (((unsigned long)(&lin[0])) >> 12) + 1;
	TH_expected_translate_pn[2] = 0xb0000;
	check_good("b");

	TH_expected_translate_calls = 2;
	TH_expected_translate_pn[2] = 0x0;
	check_bad("cdef");

	TH_expected_translate_calls = 3;
	TH_translate_handlers[2] = TH_translate_fail;
	TH_expected_translate_pn[2] = 0xb0000;
	check_bad("b");
	TH_expected_translate_calls = 2;
	TH_translate_handlers[1] = TH_translate_fail;
	check_bad("b");

	/* Check that walker respects permissions */
	make_list("a");
	TH_expected_translate_calls = 2;
	TH_translate_handlers[0] = TH_translate_passthrough;
	TH_translate_handlers[1] = TH_translate_passthrough;
	TH_translate_handlers[2] = TH_translate_fail;
	TH_expected_translate_pn[0] = ((unsigned long)(&lin[0])) >> 12;
	TH_expected_translate_pn[1] = 0xa0000;
	for (i = 0; i <= 0xf; i++) {
		TH_xwru_mask = i | 2;
		check_good("a");
		TH_xwru_mask = i & ~2;
		check_bad("a");
	};
	puts("TEST PASSED");
	return 0;
}

