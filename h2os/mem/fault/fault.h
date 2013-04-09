/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2OS_FAULT_H
#define H2OS_FAULT_H 1

void h2os_mm_fault_nopage(mm, vaddr);
void h2os_mm_fault_read(mm, vaddr);
void h2os_mm_fault_write(mm, vaddr);
void h2os_mm_fault_execute(mm, vaddr);

#endif
