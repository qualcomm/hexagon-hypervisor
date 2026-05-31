/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_VARADIX_H
#define H2K_VARADIX_H 1

/* Entry format is H2K_linear_fmt_t.low (see h2_common_linear.h):
 *   [23: 0] ppn      (24 bits)
 *   [26:24] cccc      (3 bits)
 *   [27]    weak_ccc  (1 bit)
 *   [31:28] xwru      (4 bits)
 */
#include <h2_common_linear.h>

#define VARADIX_PPN_BITS         __builtin_popcount((u32_t)(H2K_linear_fmt_t){.ppn = ~0u}.ppn)

#define VARADIX_CCCC_OFFSET      VARADIX_PPN_BITS
#define VARADIX_CCCC_MASK        ((u32_t)(H2K_linear_fmt_t){.cccc     = ~0u}.cccc)
#define VARADIX_WEAK_CCC_OFFSET  (VARADIX_CCCC_OFFSET     + __builtin_popcount(VARADIX_CCCC_MASK))
#define VARADIX_WEAK_CCC_ON      ((u32_t)(H2K_linear_fmt_t){.weak_ccc = ~0u}.weak_ccc)
#define VARADIX_XWRU_OFFSET      (VARADIX_WEAK_CCC_OFFSET + __builtin_popcount(VARADIX_WEAK_CCC_ON))
#define VARADIX_XWRU_MASK        ((u32_t)(H2K_linear_fmt_t){.xwru     = ~0u}.xwru)

#define VARADIX_PN_BITS          22

#include <translate.h>

_Static_assert(VARADIX_XWRU_OFFSET + __builtin_popcount(VARADIX_XWRU_MASK) == 32,
    "varadix entry fields must fill a u32_t");

H2K_translation_t H2K_varadix_translate(H2K_translation_t in, H2K_asid_entry_t info) IN_SECTION(".text.mem.varadix");

#endif
