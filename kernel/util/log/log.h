/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_LOG_H
#define H2K_LOG_H 1

#ifdef H2K_LOGBUF

#define H2K_log(...) H2K_log_print(__VA_ARGS__)
#define H2K_log_string(S) H2K_do_log_string(S)

u32_t H2K_write(u32_t fd, const u8_t *buf, u32_t len) IN_SECTION(".text.util.log");
u32_t H2K_do_log_string(const char *string) IN_SECTION(".text.util.log");
u32_t H2K_log_print(const char *format, ...) IN_SECTION(".text.util.log");
char *H2K_logbuf_alloc(u32_t count) IN_SECTION(".text.util.log");
char *H2K_log_init(void) IN_SECTION(".text.util.log");

#else

#define H2K_log(...)
#define H2K_log_string(S)

#endif

#endif
