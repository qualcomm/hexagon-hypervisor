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

#ifndef __ATOMIC_OPS_H__
#define __ATOMIC_OPS_H__

typedef unsigned int atomic_plain_word_t;

/*-------------------------------------------------------------------------*/
                        /* Atomic Ops API. */

//  This file scares me.  MAKE IT GO AWAY!!!

/*
 * IMPORTANT!
 * If you plan to change the structure atomic_word_t, please add the new
 * elements after value. For more information read the comment in
 * arch/arm/libs/atomic_ops/v5/src/arm_atomic_ops.spp:66
 */

typedef struct {
    volatile atomic_plain_word_t value;
} atomic_word_t;

#define ATOMIC_INIT(i)  { (i) }

static inline void
atomic_init(atomic_word_t *a, atomic_plain_word_t v)
{
    a->value = v;
}

#if defined(ARCH_ARM) && defined(ARCH_VER) && (ARCH_VER < 6) && \
         (!defined(__ATOMIC_OPS_IN_KERNEL__) || defined(MACHINE_SMP))

/* 
 * If it is ARMv4/v5, the function declarations may change
 * and are defined in the arch specific header file,
 * as some of then can't be declared static because of
 * the assembler implementation.
 */

#else 

static atomic_plain_word_t atomic_set(atomic_word_t* target,
                                      atomic_plain_word_t value);

/* Bitwise operations. */
static void atomic_and(atomic_word_t* target,
                       atomic_plain_word_t mask);
static atomic_plain_word_t atomic_and_return(atomic_word_t* target,
                                             atomic_plain_word_t mask);

static void atomic_or(atomic_word_t* target,
                      atomic_plain_word_t mask);
static atomic_plain_word_t atomic_or_return(atomic_word_t* target,
                                            atomic_plain_word_t mask);

static void atomic_xor(atomic_word_t* target,
                       atomic_plain_word_t mask);
static atomic_plain_word_t atomic_xor_return(atomic_word_t* target,
                                             atomic_plain_word_t mask);

/* Bitfield operations. */

static void atomic_set_bit(atomic_word_t *target, unsigned long bit);
static void atomic_clear_bit(atomic_word_t *target, unsigned long bit);
static void atomic_change_bit(atomic_word_t *target, unsigned long bit);

/* Arithmetic operations. */
static void atomic_add(atomic_word_t *target, atomic_plain_word_t v);
static atomic_plain_word_t atomic_add_return(atomic_word_t *target,
                                             atomic_plain_word_t v);

static void atomic_sub(atomic_word_t *target, atomic_plain_word_t v);
static atomic_plain_word_t atomic_sub_return(atomic_word_t *target,
                                             atomic_plain_word_t v);

static void atomic_inc(atomic_word_t *target);
static atomic_plain_word_t atomic_inc_return(atomic_word_t *target);

static void atomic_dec(atomic_word_t *target);
static atomic_plain_word_t atomic_dec_return(atomic_word_t *target);

/* General operations. */

static int atomic_compare_and_set(atomic_word_t *target,
                                  atomic_plain_word_t old_val,
                                  atomic_plain_word_t new_val);

/* Memory barrier operations. */

static void atomic_barrier_write(void);
static void atomic_barrier_write_smp(void);
static void atomic_barrier_read(void);
static void atomic_barrier_read_smp(void);
static void atomic_barrier(void);
static void atomic_barrier_smp(void);

#endif

/*---------------------------------------------------------------------------*/

/* Architecture independent definitions. */

static atomic_plain_word_t atomic_read(atomic_word_t target);
static void atomic_compiler_barrier(void);

static inline atomic_plain_word_t
atomic_read(atomic_word_t target)
{
    return target.value;
}

static inline void atomic_compiler_barrier()
{
#if defined(__RVCT__) || defined(__RVCT_GNU__)
    __memory_changed();
#elif defined(__ADS__)
    __asm("");
#else
    asm volatile (""::: "memory");
#endif
}

/* Architecture dependent definitions. */
#include "arch_atomic_ops.h"

#endif /* __ATOMIC_OPS_H__ */
