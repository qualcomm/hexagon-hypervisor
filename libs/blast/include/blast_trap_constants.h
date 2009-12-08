/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * blast_trap_constants
 * Just the constants we need for traps into the kernel
 *
 * WARNING: THIS IS INCLUDED BY ASSEMBLY FILES! DEFINES ONLY
 */

#ifndef BLAST_TRAP_CONSTANTS_H
#define BLAST_TRAP_CONSTANTS_H 1

#define BLAST_TRAP_INTWAIT 1
#define BLAST_TRAP_FUTEX_WAIT 2
#define BLAST_TRAP_FUTEX_WAKE 3
#define BLAST_TRAP_THREAD_CREATE 4
#define BLAST_TRAP_THREAD_STOP 5
#define BLAST_TRAP_CPUTIME 6
#define BLAST_TRAP_THREAD_ID 7
#define BLAST_TRAP_REGISTER_FASTINT 8
#define BLAST_TRAP_PRIO_SET 9
#define BLAST_TRAP_PRIO_GET 10
#define BLAST_TRAP_YIELD 12
#define BLAST_TRAP_MASK_PRIOS_ABOVE 13
#define BLAST_TRAP_ERROR_GET 14
#define BLAST_TRAP_RESTART_THREAD 15
#define BLAST_TRAP_GET_PCYCLES 16
#define BLAST_TRAP_SET_PREFETCH 17
#define BLAST_TRAP_SET_TID 18
#define BLAST_TRAP_GET_TID 19
#define BLAST_TRAP_CONFIG 30

#endif

