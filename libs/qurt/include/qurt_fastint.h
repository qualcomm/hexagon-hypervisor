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

unsigned int qurt_fastint_register(int intno, void (*fn)(int));

unsigned int qurt_fastint_deregister(int intno);

unsigned int qurt_isr_register(int intno, void (*fn)(int));

unsigned int qurt_isr_deregister(int intno);

#endif /* QURT_FASTINT_H */
