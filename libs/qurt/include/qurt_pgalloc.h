/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_PGALLOC_H
#define QURT_PGALLOC_H 1

#include "qurt_types.h"

/* 
 * allocate section from freelist 
 * Note that "addr" is arbitrary unit, we typically use vpn/ppn.
 * You can remove from freelist by allocating stuff and never freeing it.
 */
unsigned int qurt_pgalloc(struct qurt_freelist_node **ptr,unsigned int addr,unsigned int size,unsigned int alignhint);

/* 
 * allocate section from freelist 
 * Note that "addr" is arbitrary unit, we typically use vpn/ppn.
 * You can add to freelist by freeing stuff that was never allocated before.
 */
void qurt_pgfree(struct qurt_freelist_node **ptr,unsigned int addr,unsigned int size);

#endif
