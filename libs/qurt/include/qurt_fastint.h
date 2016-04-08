/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_FASTINT_H
#define QURT_FASTINT_H

/**
  @file qurt_fastint.h
  @brief <description>
*/

/* 
 * EJP: qurt fastints return void, H2 fastints return an int on whether to
 * whack interrupt automatically.  Maybe change H2 interface? Another level
 * of indirection?
 */
unsigned int qurt_fastint_register(int intno, int (*fn)(int));

unsigned int qurt_fastint_deregister(int intno);

static inline unsigned int qurt_isr_register(int intno, int (*fn)(int)) { return qurt_fastint_register(intno,fn); }

static inline unsigned int qurt_isr_deregister(int intno) { return qurt_fastint_deregister(intno); }

#endif /* QURT_FASTINT_H */
