/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_ATOMIC_H
#define H2K_ATOMIC_H 1

static inline u32_t H2K_atomic_setbit(u32_t *word, u32_t bit)
{
	u32_t t;
	asm (	"// atomic set bit\n"
		"1: %0 = memw_locked(%3)\n"
		" { p1 = tstbit(%0,%2)\n"
		"   %0 = setbit(%0,%2) }\n"
		"   memw_locked(%3,p0) = %0\n"
		" { if (!p0) jump 1b\n"
		"   %0 = mux(p1,#0,#1) }\n"
		: "=&r"(t),"+m"(*word)
		: "r"(bit),"r"(word)
		: "p0","p1");
	return t;
}

static inline u32_t H2K_atomic_clrbit(u32_t *word, u32_t bit)
{
	u32_t t;
	asm (	"// atomic clear bit\n"
		"1: %0 = memw_locked(%3)\n"
		" { p1 = tstbit(%0,%2)\n"
		"   %0 = clrbit(%0,%2) }\n"
		"   memw_locked(%3,p0) = %0\n"
		" { if (!p0) jump 1b\n"
		"   %0 = mux(p1,#1,#0) }\n"
		: "=&r"(t),"+m"(*word)
		: "r"(bit),"r"(word)
		: "p0","p1");
	return t;
}

static inline u32_t H2K_atomic_swap(u32_t *word, u32_t val)
{
	u32_t t;
	asm (	"// atomic swap\n"
		"1: %0 = memw_locked(%3)\n"
		"   memw_locked(%3,p0) = %2\n"
		"   if (!p0) jump 1b\n"
		: "=&r"(t),"+m"(*word)
		: "r"(val),"r"(word)
		: "p0");
	return t;
}

/* if ((old = *word) == expect) *word = val; return old */
static inline u32_t H2K_atomic_compare_swap(u32_t *word, u32_t expect, u32_t val) {

	u32_t t;

	asm ( "// atomic compare and swap\n"
				"1:	%0 = memw_locked(%2)\n"
				"	{ p0 = cmp.eq(%0, %3)\n"
				"	  if (!p0.new) jump:nt 2f }\n"
				"	memw_locked(%2, p0) = %4\n"
				"	if (!p0) jump 1b\n"
				"2:\n"
				: "=&r"(t), "+m"(*word)
				: "r" (word), "r" (expect), "r" (val)
				: "p0"
				);
	return t;
}

static inline u32_t H2K_atomic_insert(u32_t *word, u32_t val, u32_t width, u32_t offset)
{
	u32_t t;
	union {
		u64_t insert_info;
		struct {
			u32_t offset;
			u32_t width;
		};
	} x;
	x.width = width;
	x.offset = offset;
	asm (	"// atomic insert\n"
		"1: %0 = memw_locked(%3)\n"
		"   %0 = insert(%2,%4)\n"
		"   memw_locked(%3,p0) = %0\n"
		"   if (!p0) jump 1b\n"
		: "=&r"(t),"+m"(*word)
		: "r"(val),"r"(word),"r"(x.insert_info)
		: "p0");
	return t;
}

static inline u32_t H2K_atomic_add(u32_t *word, u32_t val) {

	u32_t t;

	asm ("// atomic add\n"
			 "1: %0 = memw_locked(%3)\n"
			 "   %0 = add(%0, %2)\n"
			 "   memw_locked(%3, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t),"+m"(*word)
			 : "r"(val),"r"(word)
			 : "p0");

	return t;
}

static inline u32_t H2K_atomic_or(u32_t *word, u32_t val) {

	u32_t t;

	asm ("// atomic or\n"
			 "1: %0 = memw_locked(%3)\n"
			 "   %0 = or(%0, %2)\n"
			 "   memw_locked(%3, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t),"+m"(*word)
			 : "r"(val),"r"(word)
			 : "p0");

	return t;
}

static inline u64_t H2K_atomic_add64(u64_t *word, u64_t val) {
	u64_t t;
	asm ("// atomic add64\n"
			 "1: %0 = memd_locked(%3)\n"
			 "   %0 = add(%0, %2)\n"
			 "   memd_locked(%3, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t),"+m"(*word)
			 : "r"(val),"r"(word)
			 : "p0");
	return t;
}

/* *_mask functions operate on (*word & mask).  Must call with other arguments
	 *already in proper bit position; other bits must be 0  */

static inline u32_t H2K_atomic_add_mask(u32_t *word, u32_t val, u32_t mask) {

	u32_t t, x;

	asm ("// atomic add mask\n"
			 "1: %0 = memw_locked(%4)\n"
			 "   %1 = and(%0, %5)\n"
			 " { %0 = xor(%0, %1)\n"
			 "   %1 = add(%1, %3) }\n"

			 "   %1 = and(%1, %5)\n"
			 "   %0 = or(%0, %1)\n"
			 "   memw_locked(%4, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t), "=&r"(x),"+m"(*word)
			 : "r"(val),"r"(word), "r"(mask)
			 : "p0");

	return x;
}

static inline u32_t H2K_atomic_max_mask(u32_t *word, u32_t val, u32_t mask) {

	u32_t t, x;

	asm ("// atomic max mask\n"
			 "1: %0 = memw_locked(%4)\n"
			 "   %1 = and(%0, %5)\n"
			 " { %0 = xor(%0, %1)\n"
			 "   %1 = max(%1, %3) }\n"

			 "   %0 = or(%0, %1)\n"
			 "   memw_locked(%4, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t), "=&r"(x),"+m"(*word)
			 : "r"(val),"r"(word), "r"(mask)
			 : "p0");

	return x;
}

static inline u32_t H2K_atomic_compare_swap_mask(u32_t *word, u32_t expect, u32_t val, u32_t mask) {

	u32_t t, x;

	asm ( "// atomic compare and swap mask\n"
				"1:	%0 = memw_locked(%3)\n"
				"   %1 = and(%0, %6)\n"
				"	{ p0 = cmp.eq(%1, %4)\n"
				"   %0 = xor(%0, %1)\n"
				"	  if (!p0.new) jump:nt 2f }\n"

				"   %0 = or(%0, %5)\n"
				"	  memw_locked(%3, p0) = %0\n"
				"	  if (!p0) jump 1b\n"
				"2:\n"
				: "=&r"(t), "=&r"(x), "+m"(*word)
				: "r"(word), "r"(expect), "r"(val), "r"(mask)
				: "p0"
				);
	return x;
}

#endif

