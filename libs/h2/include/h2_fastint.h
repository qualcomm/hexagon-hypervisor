/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_FASTINT_H
#define H2_FASTINT_H 1

void h2_register_fastint(int intno, int (*fn)(int));
void h2_deregister_fastint(int intno);

#endif
