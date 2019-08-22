/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VARADIX_H
#define H2K_VARADIX_H 1

#define VARADIX_XWRU_OFFSET 28
#define VARADIX_CCCC_OFFSET 24
#define VARADIX_PN_BITS 22

#include <translate.h>

H2K_translation_t H2K_varadix_translate(H2K_translation_t in, H2K_asid_entry_t info) IN_SECTION(".text.mem.varadix");

#endif
