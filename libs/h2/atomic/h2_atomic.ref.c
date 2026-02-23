/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/** @file h2_atomic.ref.c
 * 
 * @brief Atomic modification of memory - Implementation
 */

#include "h2_atomic.h"

atomic_u32_t h2_atomic_setbit32(atomic_u32_t *word, atomic_u32_t bit)
{
	atomic_u32_t t;
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

atomic_u32_t h2_atomic_clrbit32(atomic_u32_t *word, atomic_u32_t bit)
{
	atomic_u32_t t;
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

atomic_u32_t h2_atomic_swap32(atomic_u32_t *word, atomic_u32_t val)
{
	atomic_u32_t t;
	asm (	"// atomic swap\n"
		"1: %0 = memw_locked(%3)\n"
		"   memw_locked(%3,p0) = %2\n"
		"   if (!p0) jump 1b\n"
		: "=&r"(t),"+m"(*word)
		: "r"(val),"r"(word)
		: "p0");
	return t;
}

atomic_u32_t h2_atomic_compare_swap32(atomic_u32_t *word, atomic_u32_t expect, atomic_u32_t val) {

	atomic_u32_t t;

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

atomic_u32_t h2_atomic_insert32(atomic_u32_t *word, atomic_u32_t val, atomic_u32_t width, atomic_u32_t offset)
{
	atomic_u32_t t;
	union {
		atomic_u64_t insert_info;
		struct {
			atomic_u32_t offset;
			atomic_u32_t width;
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

atomic_u32_t h2_atomic_add32(atomic_u32_t *word, atomic_u32_t val) {
	atomic_u32_t t;
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

atomic_u32_t h2_atomic_sub32(atomic_u32_t *word, atomic_u32_t val) {
	atomic_u32_t t;
	asm ("// atomic sub\n"
			 "1: %0 = memw_locked(%3)\n"
			 "   %0 = sub(%0, %2)\n"
			 "   memw_locked(%3, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t),"+m"(*word)
			 : "r"(val),"r"(word)
			 : "p0");
	return t;
}

atomic_u32_t h2_atomic_bus32(atomic_u32_t *word, atomic_u32_t val) {
	atomic_u32_t t;
	asm ("// atomic sub\n"
			 "1: %0 = memw_locked(%3)\n"
			 "   %0 = sub(%2, %0)\n"
			 "   memw_locked(%3, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t),"+m"(*word)
			 : "r"(val),"r"(word)
			 : "p0");
	return t;
}

atomic_u32_t h2_atomic_or32(atomic_u32_t *word, atomic_u32_t val) {
	atomic_u32_t t;
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

atomic_u32_t h2_atomic_and32(atomic_u32_t *word, atomic_u32_t val) {
	atomic_u32_t t;
	asm ("// atomic and\n"
			 "1: %0 = memw_locked(%3)\n"
			 "   %0 = and(%0, %2)\n"
			 "   memw_locked(%3, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t),"+m"(*word)
			 : "r"(val),"r"(word)
			 : "p0");
	return t;
}

atomic_u32_t h2_atomic_xor32(atomic_u32_t *word, atomic_u32_t val) {
	atomic_u32_t t;
	asm ("// atomic xor\n"
			 "1: %0 = memw_locked(%3)\n"
			 "   %0 = xor(%0, %2)\n"
			 "   memw_locked(%3, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t),"+m"(*word)
			 : "r"(val),"r"(word)
			 : "p0");
	return t;
}

atomic_u64_t h2_atomic_setbit64(atomic_u64_t *word, atomic_u32_t bit)
{
	atomic_u32_t *tmpp = (atomic_u32_t *)word;
	if (bit > 32) {
		return h2_atomic_setbit32(tmpp+1,bit-32);
	} else {
		return h2_atomic_setbit32(tmpp,bit);
	}
}

atomic_u64_t h2_atomic_clrbit64(atomic_u64_t *word, atomic_u32_t bit)
{
	atomic_u32_t *tmpp = (atomic_u32_t *)word;
	if (bit > 32) {
		return h2_atomic_clrbit32(tmpp+1,bit-32);
	} else {
		return h2_atomic_clrbit32(tmpp,bit);
	}
}

atomic_u64_t h2_atomic_swap64(atomic_u64_t *word, atomic_u64_t val)
{
	atomic_u64_t t;
	asm (	"// atomic swap64\n"
		"1: %0 = memd_locked(%3)\n"
		"   memd_locked(%3,p0) = %2\n"
		"   if (!p0) jump 1b\n"
		: "=&r"(t),"+m"(*word)
		: "r"(val),"r"(word)
		: "p0");
	return t;
}

atomic_u64_t h2_atomic_compare_swap64(atomic_u64_t *word, atomic_u64_t expect, atomic_u64_t val) {

	atomic_u64_t t;

	asm ( "// atomic compare and swap64\n"
				"1:	%0 = memd_locked(%2)\n"
				"	{ p0 = cmp.eq(%0, %3)\n"
				"	  if (!p0.new) jump:nt 2f }\n"
				"	memd_locked(%2, p0) = %4\n"
				"	if (!p0) jump 1b\n"
				"2:\n"
				: "=&r"(t), "+m"(*word)
				: "r" (word), "r" (expect), "r" (val)
				: "p0"
				);
	return t;
}

atomic_u64_t h2_atomic_insert64(atomic_u64_t *word, atomic_u64_t val, atomic_u32_t width, atomic_u32_t offset)
{
	atomic_u64_t t;
	union {
		atomic_u64_t insert_info;
		struct {
			atomic_u32_t offset;
			atomic_u32_t width;
		};
	} x;
	x.width = width;
	x.offset = offset;
	asm (	"// atomic insert64\n"
		"1: %0 = memd_locked(%3)\n"
		"   %0 = insert(%2,%4)\n"
		"   memd_locked(%3,p0) = %0\n"
		"   if (!p0) jump 1b\n"
		: "=&r"(t),"+m"(*word)
		: "r"(val),"r"(word),"r"(x.insert_info)
		: "p0");
	return t;
}

atomic_u64_t h2_atomic_add64(atomic_u64_t *word, atomic_u64_t val) {
	atomic_u64_t t;
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

atomic_u64_t h2_atomic_sub64(atomic_u64_t *word, atomic_u64_t val) {
	atomic_u64_t t;
	asm ("// atomic sub64\n"
			 "1: %0 = memd_locked(%3)\n"
			 "   %0 = sub(%0, %2)\n"
			 "   memd_locked(%3, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t),"+m"(*word)
			 : "r"(val),"r"(word)
			 : "p0");
	return t;
}

atomic_u64_t h2_atomic_bus64(atomic_u64_t *word, atomic_u64_t val) {
	atomic_u64_t t;
	asm ("// atomic bus64\n"
			 "1: %0 = memd_locked(%3)\n"
			 "   %0 = sub(%2, %0)\n"
			 "   memd_locked(%3, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t),"+m"(*word)
			 : "r"(val),"r"(word)
			 : "p0");
	return t;
}

atomic_u64_t h2_atomic_or64(atomic_u64_t *word, atomic_u64_t val) {
	atomic_u64_t t;
	asm ("// atomic or64\n"
			 "1: %0 = memd_locked(%3)\n"
			 "   %0 = or(%0, %2)\n"
			 "   memd_locked(%3, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t),"+m"(*word)
			 : "r"(val),"r"(word)
			 : "p0");
	return t;
}

atomic_u64_t h2_atomic_and64(atomic_u64_t *word, atomic_u64_t val) {
	atomic_u64_t t;
	asm ("// atomic and64\n"
			 "1: %0 = memd_locked(%3)\n"
			 "   %0 = and(%0, %2)\n"
			 "   memd_locked(%3, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t),"+m"(*word)
			 : "r"(val),"r"(word)
			 : "p0");
	return t;
}

atomic_u64_t h2_atomic_xor64(atomic_u64_t *word, atomic_u64_t val) {
	atomic_u64_t t;
	asm ("// atomic xor64\n"
			 "1: %0 = memd_locked(%3)\n"
			 "   %0 = xor(%0, %2)\n"
			 "   memd_locked(%3, p0) = %0\n"
			 "   if !p0 jump 1b\n"
			 : "=&r"(t),"+m"(*word)
			 : "r"(val),"r"(word)
			 : "p0");
	return t;
}
