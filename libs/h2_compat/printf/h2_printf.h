/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2_PRINTF_H
#define H2_PRINTF_H 1

/** @file h2_printf.h
 * @brief C library printf, but with a lock
 */

int h2_printf(const char *fmt, ...);

#endif

