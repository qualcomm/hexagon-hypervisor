/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * h2_trap_constants
 * Just the constants we need for traps into the kernel
 *
 * WARNING: THIS IS INCLUDED BY ASSEMBLY FILES! DEFINES ONLY
 */

#ifndef H2_TRAP_CONSTANTS_H
#define H2_TRAP_CONSTANTS_H 1

#define H2_TRAP_INTWAIT 1
#define H2_TRAP_FUTEX_WAIT 2
#define H2_TRAP_FUTEX_WAKE 3
#define H2_TRAP_THREAD_CREATE 4
#define H2_TRAP_THREAD_STOP 5
#define H2_TRAP_CPUTIME 6
#define H2_TRAP_THREAD_ID 7
#define H2_TRAP_REGISTER_FASTINT 8
#define H2_TRAP_PRIO_SET 9
#define H2_TRAP_PRIO_GET 10
#define H2_TRAP_YIELD 12
#define H2_TRAP_MASK_PRIOS_ABOVE 13
#define H2_TRAP_ERROR_GET 14
#define H2_TRAP_RESTART_THREAD 15
#define H2_TRAP_GET_PCYCLES 16
#define H2_TRAP_SET_PREFETCH 17
#define H2_TRAP_SET_TID 18
#define H2_TRAP_GET_TID 19
#define H2_TRAP_CONFIG 30

#endif

