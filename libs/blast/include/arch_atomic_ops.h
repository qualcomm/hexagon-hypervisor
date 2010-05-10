/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/************************ COPYRIGHT NOTICE ***********************************/

/* All data and information contained in or disclosed by this                */
/* document is confidential and proprietary information of                   */
/* QUALCOMM, Inc and all rights therein are expressly reserved.              */
/* By accepting this material the recipient agrees that this                 */
/* material and the information contained therein is held in                 */
/* confidence and in trust and will not be used, copied,                     */
/* reproduced in whole or in part, nor its contents revealed in              */
/* any manner to others without the express written permission               */
/* of QUALCOMM, Inc.                                                         */
/*****************************************************************************/

#ifndef __ARCH_ATOMIC_OPS_H__
#define __ARCH_ATOMIC_OPS_H__

///* Sanity check to ensure the smp flag is set in machines.py */
//#if defined(__ATOMIC_OPS_IN_KERNEL__) && !defined(MACHINE_SMP) && CONFIG_NUM_UNITS > 1
//#error CONFIG_NUM_UNITS > 1 but smp not defined in machines.py.
//#endif

static inline atomic_plain_word_t
atomic_set(atomic_word_t* target, atomic_plain_word_t value)
{
    unsigned long tmp;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       memw_locked(%1, p0) = %2\n"
        "       if !p0 jump 1b\n"
        : "=&r" (tmp)
        : "r" (&target->value), "r" (value)
        : "p0");
    return value;
}

/* Bitwise operations. */
static inline void
atomic_and(atomic_word_t* target, atomic_plain_word_t mask)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = and(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value), "r" (mask)
        : "p0");
}

static inline atomic_plain_word_t
atomic_and_return(atomic_word_t* target, atomic_plain_word_t mask)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = and(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value), "r" (mask)
        : "p0");

    return result;
}

static inline void
atomic_or(atomic_word_t* target, atomic_plain_word_t mask)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = or(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value), "r" (mask)
        : "p0");
}

static inline atomic_plain_word_t
atomic_or_return(atomic_word_t* target, atomic_plain_word_t mask)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = or(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value), "r" (mask)
        : "p0");

    return result;
}

static inline void
atomic_xor(atomic_word_t* target, atomic_plain_word_t mask)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = xor(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value), "r" (mask)
        : "p0");
}

static inline atomic_plain_word_t
atomic_xor_return(atomic_word_t* target, atomic_plain_word_t mask)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = xor(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value), "r" (mask)
        : "p0");

    return result;
}

/* Bitfield operations. */

static inline void
atomic_set_bit(atomic_word_t *target, unsigned long bit)
{
    int word_ = bit / (sizeof(atomic_word_t) * 8);
    atomic_plain_word_t mask = 1 << (bit % (sizeof(atomic_word_t) * 8));
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = or(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target[word_].value), "r" (mask)
        : "p0");
}

static inline void
atomic_clear_bit(atomic_word_t *target, unsigned long bit)
{
    int word_ = bit / (sizeof(atomic_word_t) * 8);
    atomic_plain_word_t mask = 1 << (bit % (sizeof(atomic_word_t) * 8));
    atomic_plain_word_t result;

    mask = ~mask;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = and(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target[word_].value), "r" (mask)
        : "p0");
}

static inline void
atomic_change_bit(atomic_word_t *target, unsigned long bit)
{
    int word_ = bit / (sizeof(atomic_word_t) * 8);
    atomic_plain_word_t mask = 1 << (bit % (sizeof(atomic_word_t) * 8));
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = xor(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target[word_].value), "r" (mask)
        : "p0");
}

/* Arithmetic operations. */
static inline void
atomic_add(atomic_word_t *target, atomic_plain_word_t v)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = add(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value), "r" (v)
        : "p0");
}

static inline atomic_plain_word_t
atomic_add_return(atomic_word_t *target, atomic_plain_word_t v)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = add(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value), "r" (v)
        : "p0");

    return result;
}

static inline void
atomic_sub(atomic_word_t *target, atomic_plain_word_t v)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = sub(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value), "r" (v)
        : "p0");
}

static inline atomic_plain_word_t
atomic_sub_return(atomic_word_t *target, atomic_plain_word_t v)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = sub(%0, %2)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value), "r" (v)
        : "p0");

    return result;
}

static inline void
atomic_inc(atomic_word_t *target)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = add(%0, #1)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value)
        : "p0");
}

static inline atomic_plain_word_t
atomic_inc_return(atomic_word_t *target)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = add(%0, #1)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value)
        : "p0");

    return result;
}

static inline void
atomic_dec(atomic_word_t *target)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = add(%0, #-1)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value)
        : "p0");
}

static inline atomic_plain_word_t
atomic_dec_return(atomic_word_t *target)
{
    atomic_plain_word_t result;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       %0 = add(%0, #-1)\n"
        "       memw_locked(%1, p0) = %0\n"
        "       if !p0 jump 1b\n"
        : "=&r" (result)
        : "r" (&target->value)
        : "p0");

    return result;
}

/* General operations. */

static inline int
atomic_compare_and_set(atomic_word_t *target,
                       atomic_plain_word_t old_val,
                       atomic_plain_word_t new_val)
{
    atomic_plain_word_t current_val;

    __asm__ __volatile__(
        "1:     %0 = memw_locked(%1)\n"
        "       p0 = cmp.eq(%0, %2)\n"
        "       if !p0 jump 2f\n"
        "       memw_locked(%1, p0) = %3\n"
        "       if !p0 jump 1b\n"
        "2:\n"
        : "=&r" (current_val)
        : "r" (&target->value), "r" (old_val), "r" (new_val)
        : "p0");

    return current_val == old_val;
}

static inline void
atomic_barrier(void)
{
    __asm__ __volatile__ (
        ""
        :
        :
        :
        "memory");
}

static inline void
atomic_barrier_write(void)
{
    atomic_barrier();
}

static inline void
atomic_barrier_write_smp(void)
{
    atomic_barrier();
}

static inline void
atomic_barrier_read(void)
{
    atomic_barrier();
}

static inline void
atomic_barrier_read_smp(void)
{
    atomic_barrier();
}

static inline void
atomic_barrier_smp(void)
{
    atomic_barrier();
}

#endif /* _ARCH_ATOMIC_OPS_H__ */
