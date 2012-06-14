/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_TRANSLATE_H
#define H2K_TRANSLATE_H 1

s32_t H2K_translate(u32_t addr, u32_t pmap, translation_type type, u32_t *result) IN_SECTION(".text.mem.translate");

#endif
