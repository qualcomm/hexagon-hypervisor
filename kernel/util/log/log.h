/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#ifndef H2K_LOG_H
#define H2K_LOG_H 1

#define H2K_LOG_ERR  0
#define H2K_LOG_WARN 1
#define H2K_LOG_INFO 2
#define H2K_LOG_DBG  3

/* Override at build time with -DH2K_LOG_LEVEL=H2K_LOG_WARN etc. */
#ifndef H2K_LOG_LEVEL
#define H2K_LOG_LEVEL H2K_LOG_DBG
#endif

#ifdef H2K_LOGBUF

s32_t H2K_write(u32_t fd, const u8_t *buf, u32_t len) IN_SECTION(".text.util.log");
u32_t H2K_do_log_string(const char *string) IN_SECTION(".text.util.log");
s32_t H2K_log_print(u32_t level, const char *file, u32_t line, const char *format, ...)
	IN_SECTION(".text.util.log") __attribute__((format(printf, 4, 5)));
char *H2K_logbuf_alloc(u32_t htid, u32_t count) IN_SECTION(".text.util.log");
char *H2K_log_init(void) IN_SECTION(".text.util.log");

u32_t H2K_log_once(u32_t *flag) IN_SECTION(".text.util.log");
u32_t H2K_log_once_ht(u32_t *flags) IN_SECTION(".text.util.log");
u32_t H2K_log_throttle(u64_t *last, u64_t interval) IN_SECTION(".text.util.log");
u32_t H2K_log_throttle_ht(u64_t *last, u64_t interval) IN_SECTION(".text.util.log");

#define H2K_log_at(level, ...) H2K_log_print((level), __FILE__, __LINE__, __VA_ARGS__)
#define H2K_log_err(...)  H2K_log_at(H2K_LOG_ERR,  __VA_ARGS__)
#define H2K_log_warn(...) H2K_log_at(H2K_LOG_WARN, __VA_ARGS__)
#define H2K_log_info(...) H2K_log_at(H2K_LOG_INFO, __VA_ARGS__)
#define H2K_log_dbg(...)  H2K_log_at(H2K_LOG_DBG,  __VA_ARGS__)
#define H2K_log(...)      H2K_log_dbg(__VA_ARGS__)
#define H2K_log_string(S) H2K_do_log_string(S)

#define H2K_log_once(...) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log(__VA_ARGS__); } while (0)
#define H2K_log_once_err(...) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log_err(__VA_ARGS__); } while (0)
#define H2K_log_once_warn(...) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log_warn(__VA_ARGS__); } while (0)
#define H2K_log_once_info(...) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log_info(__VA_ARGS__); } while (0)
#define H2K_log_once_ht(...) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log(__VA_ARGS__); } while (0)
#define H2K_log_once_ht_err(...) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log_err(__VA_ARGS__); } while (0)
#define H2K_log_once_ht_warn(...) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log_warn(__VA_ARGS__); } while (0)
#define H2K_log_once_ht_info(...) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log_info(__VA_ARGS__); } while (0)
#define H2K_log_throttle(interval, ...) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log(__VA_ARGS__); } while (0)
#define H2K_log_throttle_err(interval, ...) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log_err(__VA_ARGS__); } while (0)
#define H2K_log_throttle_warn(interval, ...) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log_warn(__VA_ARGS__); } while (0)
#define H2K_log_throttle_info(interval, ...) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log_info(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht(interval, ...) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht_err(interval, ...) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log_err(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht_warn(interval, ...) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log_warn(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht_info(interval, ...) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log_info(__VA_ARGS__); } while (0)

#else
#ifdef H2K_LOG_PRINTF
#include <stdio.h>
#include <stdlib.h>

#define H2K_log_err(fmt, ...)   printf("[E] " fmt, ##__VA_ARGS__)
#define H2K_log_warn(fmt, ...)  printf("[W] " fmt, ##__VA_ARGS__)
#define H2K_log_info(fmt, ...)  printf("[I] " fmt, ##__VA_ARGS__)
#define H2K_log_dbg(fmt, ...)   printf("[D] " fmt, ##__VA_ARGS__)
#define H2K_log(...)            H2K_log_dbg(__VA_ARGS__)
#define H2K_log_string(S)       puts(S)

#define H2K_log_once(...)               H2K_log(__VA_ARGS__)
#define H2K_log_once_err(...)           H2K_log_err(__VA_ARGS__)
#define H2K_log_once_warn(...)          H2K_log_warn(__VA_ARGS__)
#define H2K_log_once_info(...)          H2K_log_info(__VA_ARGS__)
#define H2K_log_once_ht(...)            H2K_log(__VA_ARGS__)
#define H2K_log_once_ht_err(...)        H2K_log_err(__VA_ARGS__)
#define H2K_log_once_ht_warn(...)       H2K_log_warn(__VA_ARGS__)
#define H2K_log_once_ht_info(...)       H2K_log_info(__VA_ARGS__)
#define H2K_log_throttle(interval, ...) H2K_log(__VA_ARGS__)
#define H2K_log_throttle_err(interval, ...) H2K_log_err(__VA_ARGS__)
#define H2K_log_throttle_warn(interval, ...) H2K_log_warn(__VA_ARGS__)
#define H2K_log_throttle_info(interval, ...) H2K_log_info(__VA_ARGS__)
#define H2K_log_throttle_ht(interval, ...) H2K_log(__VA_ARGS__)
#define H2K_log_throttle_ht_err(interval, ...) H2K_log_err(__VA_ARGS__)
#define H2K_log_throttle_ht_warn(interval, ...) H2K_log_warn(__VA_ARGS__)
#define H2K_log_throttle_ht_info(interval, ...) H2K_log_info(__VA_ARGS__)
#else
#define H2K_log_err(...)
#define H2K_log_warn(...)
#define H2K_log_info(...)
#define H2K_log_dbg(...)
#define H2K_log(...)
#define H2K_log_string(S)
#define H2K_log_once(...)               do {} while (0)
#define H2K_log_once_err(...)           do {} while (0)
#define H2K_log_once_warn(...)          do {} while (0)
#define H2K_log_once_info(...)          do {} while (0)
#define H2K_log_once_ht(...)            do {} while (0)
#define H2K_log_once_ht_err(...)        do {} while (0)
#define H2K_log_once_ht_warn(...)       do {} while (0)
#define H2K_log_once_ht_info(...)       do {} while (0)
#define H2K_log_throttle(interval, ...) do {} while (0)
#define H2K_log_throttle_err(interval, ...) do {} while (0)
#define H2K_log_throttle_warn(interval, ...) do {} while (0)
#define H2K_log_throttle_info(interval, ...) do {} while (0)
#define H2K_log_throttle_ht(interval, ...) do {} while (0)
#define H2K_log_throttle_ht_err(interval, ...) do {} while (0)
#define H2K_log_throttle_ht_warn(interval, ...) do {} while (0)
#define H2K_log_throttle_ht_info(interval, ...) do {} while (0)
#endif
#endif

#endif
