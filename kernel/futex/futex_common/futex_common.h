/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_FUTEX_COMMON_H
#define H2K_FUTEX_COMMON_H 1

#ifndef ASM
#include <context.h>

/* 
 * EJP: hash table aligned to it's size, so we 
 * should be able to use tableidx on the product 
 */
#define HASHVAL(X) (Q6_R_extractu_RII((((unsigned int)(X)) * FUTEX_PRIME),FUTEX_HASHBITS,32-FUTEX_HASHBITS))

void H2K_futex_hash_add_ring(H2K_thread_context **ring, H2K_thread_context *me) IN_SECTION(".text.core.futex");
H2K_thread_context *H2K_futex_hash_remove_one(pa_t lock, H2K_thread_context **ring, H2K_thread_context **pos) IN_SECTION(".text.core.futex");

#endif

#endif

