/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_SYMBOLS_H
#define H2K_SYMBOLS_H

/* Symbols defined at link time */

void __bootvm_entry();

extern void *end;
extern void *HEAP_SIZE __attribute__ ((weak));
extern void *STACK_SIZE __attribute__ ((weak));
extern void *H2K_ALLOC_HEAP_SIZE __attribute__ ((weak));  // size in words

extern void *H2K_KERNEL_NPAGES;

#endif
