/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef RUNLIST_H
#define RUNLIST_H 1

#include <context.h>
#include <max.h>

extern BLASTK_thread_context *BLASTK_runlist[MAX_PRIOS];
extern unsigned int BLASTK_runlist_valids;

#endif

