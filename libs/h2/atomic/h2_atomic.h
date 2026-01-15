/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_ATOMIC_H
#define H2_ATOMIC_H 1

/** @file h2_atomic.h
 * 
 * @brief Atomic modification of memory
 */

/** @addtogroup h2 
@{ */

typedef unsigned int atomic_u32_t;
typedef unsigned long long int atomic_u64_t;

/**
Atomically set a bit in a word.
@param[in] word		Address of a word in memory
@param[in] bit		Bit to set in the word
@returns 0 if the bit was already set, nonzero otherwise.
@dependencies None
*/
atomic_u32_t h2_atomic_setbit32(atomic_u32_t *word, atomic_u32_t bit);

/**
Atomically clear a bit in a word.
@param[in] word		Address of a word in memory
@param[in] bit		Bit to set in the word
@returns 0 if the bit was already clear, nonzero otherwise.
@dependencies None
*/
atomic_u32_t h2_atomic_clrbit32(atomic_u32_t *word, atomic_u32_t bit);

/**
Atomically swap a word in memory.
@param[in] word		Address of a word in memory
@param[in] val		New value for the word
@returns the old value of the word
@dependencies None
*/
atomic_u32_t h2_atomic_swap32(atomic_u32_t *word, atomic_u32_t val);

/**
Atomically swap a word in memory, if it matches an expected value.
@param[in] word		Address of a word in memory
@param[in] expect	Expected value that the word must match
@param[in] val		New value for the word
@returns the old value of the word
@dependencies None
*/
atomic_u32_t h2_atomic_compare_swap32(atomic_u32_t *word, atomic_u32_t expect, atomic_u32_t val);

/**
Atomically insert a bitfield into word in memory
@param[in] word		Address of a word in memory
@param[in] val		Bits to insert into the word
@param[in] width	Number of bits to insert in the word
@param[in] offset	Starting bit number for insertion
@returns the new value of the word
@dependencies None
*/
atomic_u32_t h2_atomic_insert32(atomic_u32_t *word, atomic_u32_t val, atomic_u32_t width, atomic_u32_t offset);

/**
Atomically add to a word in memory
@param[in] word		Address of a word in memory
@param[in] val		Amount to add to the word in memory
@returns the new value of the word
@dependencies None
*/
atomic_u32_t h2_atomic_add32(atomic_u32_t *word, atomic_u32_t val);

/**
Atomically subtract from a word in memory
@param[in] word		Address of a word in memory
@param[in] val		Amount to subtract from the word in memory
@returns the new value of the word
@dependencies None
*/
atomic_u32_t h2_atomic_sub32(atomic_u32_t *word, atomic_u32_t val);

/**
Atomically subtract a word in memory from a value
@param[in] word		Address of a word in memory
@param[in] val		Amount to subtract the word in memory from
@returns the new value of the word
@dependencies None
*/
atomic_u32_t h2_atomic_bus32(atomic_u32_t *word, atomic_u32_t val);

/**
Atomically inclusive or a word in memory with a value
@param[in] word		Address of a word in memory
@param[in] val		Value to OR with the word in memory
@returns the new value of the word
@dependencies None
*/
atomic_u32_t h2_atomic_or32(atomic_u32_t *word, atomic_u32_t val);

/**
Atomically logical AND a word in memory with a value
@param[in] word		Address of a word in memory
@param[in] val		Value to AND with the word in memory
@returns the new value of the word
@dependencies None
*/
atomic_u32_t h2_atomic_and32(atomic_u32_t *word, atomic_u32_t val);

/**
Atomically exclusive or a word in memory with a value
@param[in] word		Address of a word in memory
@param[in] val		Value to XOR with the word in memory
@returns the new value of the word
@dependencies None
*/
atomic_u32_t h2_atomic_xor32(atomic_u32_t *word, atomic_u32_t val);

/**
Atomically set a bit in a double word.
@param[in] word		Address of a word in memory
@param[in] bit		Bit to set in the word
@returns 0 if the bit was already set, nonzero otherwise.
@dependencies None
*/
atomic_u64_t h2_atomic_setbit64(atomic_u64_t *word, atomic_u32_t bit);

/**
Atomically clear a bit in a word.
@param[in] word		Address of a word in memory
@param[in] bit		Bit to set in the word
@returns 0 if the bit was already clear, nonzero otherwise.
@dependencies None
*/
atomic_u64_t h2_atomic_clrbit64(atomic_u64_t *word, atomic_u32_t bit);

/**
Atomically swap a word in memory.
@param[in] word		Address of a word in memory
@param[in] val		New value for the word
@returns the old value of the word
@dependencies None
*/
atomic_u64_t h2_atomic_swap64(atomic_u64_t *word, atomic_u64_t val);

/**
Atomically swap a word in memory, if it matches an expected value.
@param[in] word		Address of a word in memory
@param[in] expect	Expected value that the word must match
@param[in] val		New value for the word
@returns the old value of the word
@dependencies None
*/
atomic_u64_t h2_atomic_compare_swap64(atomic_u64_t *word, atomic_u64_t expect, atomic_u64_t val);

/**
Atomically insert a bitfield into word in memory
@param[in] word		Address of a word in memory
@param[in] val		Bits to insert into the word
@param[in] width	Number of bits to insert in the word
@param[in] offset	Starting bit number for insertion
@returns the new value of the word
@dependencies None
*/
atomic_u64_t h2_atomic_insert64(atomic_u64_t *word, atomic_u64_t val, atomic_u32_t width, atomic_u32_t offset);

/**
Atomically add to a word in memory
@param[in] word		Address of a word in memory
@param[in] val		Amount to add to the word in memory
@returns the new value of the word
@dependencies None
*/
atomic_u64_t h2_atomic_add64(atomic_u64_t *word, atomic_u64_t val);

/**
Atomically subtract from a word in memory
@param[in] word		Address of a word in memory
@param[in] val		Amount to subtract from the word in memory
@returns the new value of the word
@dependencies None
*/
atomic_u64_t h2_atomic_sub64(atomic_u64_t *word, atomic_u64_t val);

/**
Atomically subtract a word in memory from a value
@param[in] word		Address of a word in memory
@param[in] val		Amount to subtract the word in memory from
@returns the new value of the word
@dependencies None
*/
atomic_u64_t h2_atomic_bus64(atomic_u64_t *word, atomic_u64_t val);

/**
Atomically inclusive or a word in memory with a value
@param[in] word		Address of a word in memory
@param[in] val		Value to OR with the word in memory
@returns the new value of the word
@dependencies None
*/
atomic_u64_t h2_atomic_or64(atomic_u64_t *word, atomic_u64_t val);

/**
Atomically logical AND a word in memory with a value
@param[in] word		Address of a word in memory
@param[in] val		Value to AND with the word in memory
@returns the new value of the word
@dependencies None
*/
atomic_u64_t h2_atomic_and64(atomic_u64_t *word, atomic_u64_t val);

/**
Atomically exclusive or a word in memory with a value
@param[in] word		Address of a word in memory
@param[in] val		Value to XOR with the word in memory
@returns the new value of the word
@dependencies None
*/
atomic_u64_t h2_atomic_xor64(atomic_u64_t *word, atomic_u64_t val);

#endif
