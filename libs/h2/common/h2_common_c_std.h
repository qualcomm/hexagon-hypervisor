/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_COMMON_C_STD_H
#define H2_COMMON_C_STD_H 1

typedef unsigned char h2_u8_t;
typedef unsigned short h2_u16_t;
typedef unsigned int h2_u32_t;
typedef unsigned long long int h2_u64_t;

typedef signed char h2_s8_t;
typedef signed short h2_s16_t;
typedef signed int h2_s32_t;
typedef signed long long int h2_s64_t;

typedef unsigned long h2_pa_t;

#ifndef H2_IN_SECTION
#define H2_IN_SECTION(S) __attribute__((section(S)))
#endif

#define H2_ALIGN_UP(X, SIZ) (((X) + ((SIZ) - 1)) & (-(SIZ)))

#endif
