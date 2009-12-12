/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_RING_TEST_HARNESS_H
#define H2K_RING_TEST_HARNESS_H 1

#include <ring.h>

H2K_ringnode_t *getnode(const char c);
void checkring(H2K_ringnode_t *head, const char *str);
H2K_ringnode_t *makering(const char *str);

#endif

