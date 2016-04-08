/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_FUTEX_H
#define H2K_FUTEX_H 1

#define FUTEX_HASHBITS 6
#define FUTEX_HASHSIZE (1<<FUTEX_HASHBITS)

#ifdef ASM
#define FUTEX_PRIME 2654435761
#else
#define FUTEX_PRIME 2654435761UL
#endif

#include <futex_common.h>
#include <futex_pi.h>
#include <futex_classic.h>
#include <futex_misc.h>

#endif
