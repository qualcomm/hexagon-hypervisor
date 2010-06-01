/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_CACHE_H
#define H2K_CACHE_H 1

void H2K_cache_l2_cleaninv() IN_SECTION(".text.misc.cache");
void H2K_cache_d_cleaninv() IN_SECTION(".text.misc.cache");
void H2K_cache_d_clean() IN_SECTION(".text.misc.cache");
void H2K_cache_i_inv() IN_SECTION(".text.misc.cache");

#endif

