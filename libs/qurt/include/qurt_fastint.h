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
static inline unsigned int qurt_fastint_register(int intno, void (*fn)(int))
{
	h2_register_fastint(intno+32,(void *)fn);
	return QURT_EOK;
}

static inline unsigned int qurt_fastint_deregister(int intno)
{
	h2_deregister_fastint(intno+32);
	return QURT_EOK;
}

static inline unsigned int qurt_isr_register(int intno, void (*fn)(int)) { return qurt_fastint_register(intno+32,fn); }

static inline unsigned int qurt_isr_deregister(int intno) { return qurt_fastint_deregister(intno+32); }

#endif /* QURT_FASTINT_H */
