/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#define WRITE_BUFSIZE (1024*256)
char H2_ANGEL_write_buf[WRITE_BUFSIZE] __attribute__((aligned(64))) = { 0 };
unsigned int H2_ANGEL_write_buf_idx = 0;
const unsigned int H2_ANGEL_write_buf_size = WRITE_BUFSIZE;

