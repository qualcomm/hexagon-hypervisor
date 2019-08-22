/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef QURT_S5ALLOC_H
#define QURT_S5ALLOC_H 1

typedef int qurt_s5id_t;

qurt_s5id_t qurt_s5_create(unsigned int elementsize, void (*more_mem_ptr)(void *opaque, qurt_s5id_t s5id, unsigned int elementsize), void *opaque);
int qurt_s5_feed(qurt_s5id_t id, void *mem, unsigned int memsize);
unsigned int qurt_s5_elementsize(qurt_s5id_t id);
void *qurt_s5_alloc(qurt_s5id_t id);
int qurt_s5_free(qurt_s5id_t id, void *mem);

#endif
