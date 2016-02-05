/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_PAGEWALK_H
#define H2K_PAGEWALK_H 1

#include <c_std.h>
#include <tlbfmt.h>
#include <context.h>
#include <physread.h>
#include <translate.h>
#include <globals.h>
#include <h2_common_pagewalk.h>

H2K_pte_t H2K_mem_pagewalk(u32_t badva, H2K_thread_context *me) IN_SECTION(".text.mem.pagewalk");

H2K_translation_t H2K_pagewalk_translate(H2K_translation_t in, H2K_asid_entry_t info) IN_SECTION(".text.mem.pagewalk");

#endif
