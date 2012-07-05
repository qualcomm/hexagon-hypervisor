/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_FUTEX_H
#define H2K_FUTEX_H 1

#define FUTEX_HASHBITS 5
#define FUTEX_HASHSIZE (1<<FUTEX_HASHBITS)

#include <futex_common.h>
#include <futex_pi.h>
#include <futex_classic.h>
#include <futex_misc.h>

#endif
