/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_ATOMIC_H
#define H2_ATOMIC_H 1

/** @file h2_mutex.h
 * 
 * @brief Atomic modification of memory
 */

/** @addtogroup h2 
@{ */

/**
Atomically set a bit in a word.
@param[in] word		Address of a word in memory
@param[in] bit		Bit to set in the word
@returns 0 if the bit was already set, nonzero otherwise.
@dependencies None
*/

static inline u32_t h2_atomic_setbit(u32_t *word, u32_t bit)
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

/**
Atomically clear a bit in a word.
@param[in] word		Address of a word in memory
@param[in] bit		Bit to set in the word
@returns 0 if the bit was already clear, nonzero otherwise.
@dependencies None
*/

static inline u32_t h2_atomic_clrbit(u32_t *word, u32_t bit)
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

/**
Atomically swap a word in memory.
@param[in] word		Address of a word in memory
@param[in] val		New value for the word
@returns the old value of the word
@dependencies None
*/

static inline u32_t h2_atomic_swap(u32_t *word, u32_t val)
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

/**
Atomically swap a word in memory, if it matches an expected value.
@param[in] word		Address of a word in memory
@param[in] expect	Expected value that the word must match
@param[in] val		New value for the word
@returns the old value of the word
@dependencies None
*/

static inline u32_t h2_atomic_compare_swap(u32_t *word, u32_t expect, u32_t val) {

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

/**
Atomically insert a bitfield into word in memory
@param[in] word		Address of a word in memory
@param[in] val		Bits to insert into the word
@param[in] width	Number of bits to insert in the word
@param[in] offset	Starting bit number for insertion
@returns the new value of the word
@dependencies None
*/

static inline u32_t h2_atomic_insert(u32_t *word, u32_t val, u32_t width, u32_t offset)
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

/**
Atomically add to a word in memory
@param[in] word		Address of a word in memory
@param[in] val		Amount to add to the word in memory
@returns the new value of the word
@dependencies None
*/

static inline u32_t h2_atomic_add(u32_t *word, u32_t val) {
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

/**
Atomically subtract from a word in memory
@param[in] word		Address of a word in memory
@param[in] val		Amount to subtract from the word in memory
@returns the new value of the word
@dependencies None
*/

static inline u32_t h2_atomic_sub(u32_t *word, u32_t val) {
	u32_t t;
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

/**
Atomically subtract a word in memory from a value
@param[in] word		Address of a word in memory
@param[in] val		Amount to subtract the word in memory from
@returns the new value of the word
@dependencies None
*/

static inline u32_t h2_atomic_bus(u32_t *word, u32_t val) {
	u32_t t;
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

/**
Atomically inclusive or a word in memory with a value
@param[in] word		Address of a word in memory
@param[in] val		Value to OR with the word in memory
@returns the new value of the word
@dependencies None
*/

static inline u32_t h2_atomic_or(u32_t *word, u32_t val) {
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

/**
Atomically logical AND a word in memory with a value
@param[in] word		Address of a word in memory
@param[in] val		Value to AND with the word in memory
@returns the new value of the word
@dependencies None
*/

static inline u32_t h2_atomic_and(u32_t *word, u32_t val) {
	u32_t t;
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

/**
Atomically exclusive or a word in memory with a value
@param[in] word		Address of a word in memory
@param[in] val		Value to XOR with the word in memory
@returns the new value of the word
@dependencies None
*/

static inline u32_t h2_atomic_xor(u32_t *word, u32_t val) {
	u32_t t;
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

#endif
