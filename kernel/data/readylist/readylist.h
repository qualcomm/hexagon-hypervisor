/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef READYLIST_H
#define READYLIST_H 1

#include <context.h>
#include <max.h>

extern BLASTK_thread_context *BLASTK_ready[MAX_PRIOS];
extern unsigned int BLASTK_ready_valids;

#endif

