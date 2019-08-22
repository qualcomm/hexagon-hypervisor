/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_SAMPLE_H
#define H2K_SAMPLE_H 1

void H2K_sample(u32_t hthread, H2K_thread_context *me) IN_SECTION(".text.prof.sample");
u32_t H2K_sample_start(H2K_thread_context *me) IN_SECTION(".text.prof.sample");
void H2K_sample_init(void) IN_SECTION(".text.prof.sample");

#endif
