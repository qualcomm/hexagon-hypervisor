/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_C_STD_H
#define H2K_C_STD_H 1

#ifndef NULL
#define NULL ((void *)0)
#endif

typedef unsigned char u8_t;
typedef unsigned short u16_t;
typedef unsigned int u32_t;
typedef unsigned long long int u64_t;

typedef signed char s8_t;
typedef signed short s16_t;
typedef signed int s32_t;
typedef signed long long int s64_t;

typedef u32_t pa_t;

#define likely(x)	(__builtin_expect(!!(x), 1))
#define unlikely(x)	(__builtin_expect(!!(x), 0))

#define IS_WORSE_THAN >

#define IN_SECTION(secname) __attribute__((section(secname)))

/*  Debug/assertion wrapper support  */

#ifdef DEBUG
#define call(fname, ...) fname##_debug(__VA_ARGS__)
#else
#define call(fname, ...) fname(__VA_ARGS__)
#endif

/* Hopefully this is defined by the compiler... */
#ifndef offsetof
#define offsetof(type,element) ((u32_t)(&(((type *)0)->element)))
#endif

/*
 * If we have a pointer "listptr" to an H2K_ringnode_t called "some_list" in a thread context,
 * thread = containerof(H2K_thread_context, some_list, listptr)
 *
 */
#define containerof(type, element, ptr) ((type *)((u8_t *)(ptr) - offsetof(type,element)))

#define H2K_LANG_IS_C 1

#endif

