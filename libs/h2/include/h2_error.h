/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_ERROR_H
#define H2_ERROR_H 1

void h2_handle_errors(void);
void h2_set_handler(int eventnum, void (*fn)(int));

#endif

