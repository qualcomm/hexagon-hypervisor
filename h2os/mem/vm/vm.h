/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2OS_VM_H
#define H2OS_VM_H 1

/* This has all the definitions for virtual memory stuff in it */
/* ASM-includable: Anything not includable by ASM must have an ifndef ASM */

#define PAGE_SHIFT 12

#define MAX_MEM_SIZE (1024*1024*512)

/* Starting VA for Kernel */
#define KERN_VA_START 0x80000000

/* KVA = PA + KERN_PHYS_OFFSET */
#define KERN_PHYS_OFFSET 0x80000000

#define KVA_TO_PA(VA) ((VA) - KERN_PHYS_OFFSET)
#define PA_TO_KVA(PA) ((PA) + KERN_PHYS_OFFSET)

#endif
