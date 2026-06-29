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
u32_t H2K_do_log_string_at(u32_t level, const char *file, u32_t line, const char *string) IN_SECTION(".text.util.log");
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
#define H2K_log_string(S)     H2K_do_log_string_at(H2K_LOG_DBG, __FILE__, __LINE__, S)
#define H2K_log_string_at(level, S) H2K_do_log_string_at((level), __FILE__, __LINE__, (S))
#define H2K_log_string_err(S)  H2K_log_string_at(H2K_LOG_ERR,  S)
#define H2K_log_string_warn(S) H2K_log_string_at(H2K_LOG_WARN, S)
#define H2K_log_string_info(S) H2K_log_string_at(H2K_LOG_INFO, S)
#define H2K_log_string_dbg(S)  H2K_log_string_at(H2K_LOG_DBG,  S)

#define H2K_log_once(...) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log(__VA_ARGS__); } while (0)
#define H2K_log_once_err(...) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log_err(__VA_ARGS__); } while (0)
#define H2K_log_once_warn(...) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log_warn(__VA_ARGS__); } while (0)
#define H2K_log_once_info(...) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log_info(__VA_ARGS__); } while (0)
#define H2K_log_once_dbg(...) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log_dbg(__VA_ARGS__); } while (0)
#define H2K_log_once_ht(...) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log(__VA_ARGS__); } while (0)
#define H2K_log_once_ht_err(...) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log_err(__VA_ARGS__); } while (0)
#define H2K_log_once_ht_warn(...) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log_warn(__VA_ARGS__); } while (0)
#define H2K_log_once_ht_info(...) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log_info(__VA_ARGS__); } while (0)
#define H2K_log_once_ht_dbg(...) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log_dbg(__VA_ARGS__); } while (0)
#define H2K_log_throttle(interval, ...) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log(__VA_ARGS__); } while (0)
#define H2K_log_throttle_err(interval, ...) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log_err(__VA_ARGS__); } while (0)
#define H2K_log_throttle_warn(interval, ...) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log_warn(__VA_ARGS__); } while (0)
#define H2K_log_throttle_info(interval, ...) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log_info(__VA_ARGS__); } while (0)
#define H2K_log_throttle_dbg(interval, ...) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log_dbg(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht(interval, ...) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht_err(interval, ...) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log_err(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht_warn(interval, ...) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log_warn(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht_info(interval, ...) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log_info(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht_dbg(interval, ...) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log_dbg(__VA_ARGS__); } while (0)

#define H2K_log_string_once(S) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log_string(S); } while (0)
#define H2K_log_string_once_err(S) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log_string_err(S); } while (0)
#define H2K_log_string_once_warn(S) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log_string_warn(S); } while (0)
#define H2K_log_string_once_info(S) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log_string_info(S); } while (0)
#define H2K_log_string_once_dbg(S) \
	do { static u32_t __f = 0; if ((H2K_log_once)(&__f)) H2K_log_string_dbg(S); } while (0)
#define H2K_log_string_once_ht(S) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log_string(S); } while (0)
#define H2K_log_string_once_ht_err(S) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log_string_err(S); } while (0)
#define H2K_log_string_once_ht_warn(S) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log_string_warn(S); } while (0)
#define H2K_log_string_once_ht_info(S) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log_string_info(S); } while (0)
#define H2K_log_string_once_ht_dbg(S) \
	do { static u32_t __f[MAX_HTHREADS] = {0}; if ((H2K_log_once_ht)(__f)) H2K_log_string_dbg(S); } while (0)
#define H2K_log_string_throttle(interval, S) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log_string(S); } while (0)
#define H2K_log_string_throttle_err(interval, S) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log_string_err(S); } while (0)
#define H2K_log_string_throttle_warn(interval, S) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log_string_warn(S); } while (0)
#define H2K_log_string_throttle_info(interval, S) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log_string_info(S); } while (0)
#define H2K_log_string_throttle_dbg(interval, S) \
	do { static u64_t __t = 0; if ((H2K_log_throttle)(&__t, (interval))) H2K_log_string_dbg(S); } while (0)
#define H2K_log_string_throttle_ht(interval, S) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log_string(S); } while (0)
#define H2K_log_string_throttle_ht_err(interval, S) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log_string_err(S); } while (0)
#define H2K_log_string_throttle_ht_warn(interval, S) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log_string_warn(S); } while (0)
#define H2K_log_string_throttle_ht_info(interval, S) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log_string_info(S); } while (0)
#define H2K_log_string_throttle_ht_dbg(interval, S) \
	do { static u64_t __t[MAX_HTHREADS] = {0}; if ((H2K_log_throttle_ht)(__t, (interval))) H2K_log_string_dbg(S); } while (0)

#else
#ifdef H2K_LOG_PRINTF
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <hw.h>

static inline u64_t H2K_printf_now(void) { return H2K_get_pcycle_reg(); }
static inline const char *H2K_printf_basename(const char *p) {
	const char *s = strrchr(p, '/'); return s ? s + 1 : p;
}
static inline u32_t H2K_printf_log_once_ht(u32_t *flags) {
	u32_t htid = get_hwtnum();
	if (flags[htid]) return 0;
	flags[htid] = 1;
	return 1;
}
static inline u32_t H2K_printf_log_throttle_ht(u64_t *last, u64_t interval) {
	u32_t htid = get_hwtnum();
	u64_t now = H2K_get_pcycle_reg();
	if (now - last[htid] < interval) return 0;
	last[htid] = now;
	return 1;
}

u32_t H2K_printf_log_once(int *flag);
u32_t H2K_printf_log_throttle(u64_t *last, u64_t interval);

static const char * const H2K_printf_level_str[] = { "ERROR", "WARN", "INFO", "DEBUG" };

#define H2K_log_at(level, file, line, fmt, ...) \
	do { if ((level) <= H2K_LOG_LEVEL) \
		printf("[ht-][%s][%llu][%s:%u] " fmt, H2K_printf_level_str[level], \
		       (unsigned long long)H2K_printf_now(), \
		       H2K_printf_basename(file), (unsigned int)(line), ##__VA_ARGS__); \
	} while (0)

#define H2K_log_err(fmt, ...)   H2K_log_at(H2K_LOG_ERR,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define H2K_log_warn(fmt, ...)  H2K_log_at(H2K_LOG_WARN, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define H2K_log_info(fmt, ...)  H2K_log_at(H2K_LOG_INFO, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define H2K_log_dbg(fmt, ...)   H2K_log_at(H2K_LOG_DBG,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define H2K_log(...)  H2K_log_dbg(__VA_ARGS__)
#define H2K_log_string(S)       H2K_log_at(H2K_LOG_DBG, __FILE__, __LINE__, "%s", S)
#define H2K_log_string_at(level, S) H2K_log_at((level), __FILE__, __LINE__, "%s", S)
#define H2K_log_string_err(S)   H2K_log_at(H2K_LOG_ERR,  __FILE__, __LINE__, "%s", S)
#define H2K_log_string_warn(S)  H2K_log_at(H2K_LOG_WARN, __FILE__, __LINE__, "%s", S)
#define H2K_log_string_info(S)  H2K_log_at(H2K_LOG_INFO, __FILE__, __LINE__, "%s", S)
#define H2K_log_string_dbg(S)   H2K_log_at(H2K_LOG_DBG,  __FILE__, __LINE__, "%s", S)

#define H2K_log_once(...)      do { static int __f = 0; if (H2K_printf_log_once(&__f)) H2K_log(__VA_ARGS__); } while (0)
#define H2K_log_once_err(...)  do { static int __f = 0; if (H2K_printf_log_once(&__f)) H2K_log_err(__VA_ARGS__); } while (0)
#define H2K_log_once_warn(...) do { static int __f = 0; if (H2K_printf_log_once(&__f)) H2K_log_warn(__VA_ARGS__); } while (0)
#define H2K_log_once_info(...) do { static int __f = 0; if (H2K_printf_log_once(&__f)) H2K_log_info(__VA_ARGS__); } while (0)
#define H2K_log_once_dbg(...)  do { static int __f = 0; if (H2K_printf_log_once(&__f)) H2K_log_dbg(__VA_ARGS__); } while (0)
#define H2K_log_once_ht(...)      do { static u32_t __f[MAX_HTHREADS] = {0}; if (H2K_printf_log_once_ht(__f)) H2K_log(__VA_ARGS__); } while (0)
#define H2K_log_once_ht_err(...)  do { static u32_t __f[MAX_HTHREADS] = {0}; if (H2K_printf_log_once_ht(__f)) H2K_log_err(__VA_ARGS__); } while (0)
#define H2K_log_once_ht_warn(...) do { static u32_t __f[MAX_HTHREADS] = {0}; if (H2K_printf_log_once_ht(__f)) H2K_log_warn(__VA_ARGS__); } while (0)
#define H2K_log_once_ht_info(...) do { static u32_t __f[MAX_HTHREADS] = {0}; if (H2K_printf_log_once_ht(__f)) H2K_log_info(__VA_ARGS__); } while (0)
#define H2K_log_once_ht_dbg(...)  do { static u32_t __f[MAX_HTHREADS] = {0}; if (H2K_printf_log_once_ht(__f)) H2K_log_dbg(__VA_ARGS__); } while (0)

#define H2K_log_throttle(interval, ...)      do { static u64_t __t = 0; if (H2K_printf_log_throttle(&__t, (interval))) H2K_log(__VA_ARGS__); } while (0)
#define H2K_log_throttle_err(interval, ...)  do { static u64_t __t = 0; if (H2K_printf_log_throttle(&__t, (interval))) H2K_log_err(__VA_ARGS__); } while (0)
#define H2K_log_throttle_warn(interval, ...) do { static u64_t __t = 0; if (H2K_printf_log_throttle(&__t, (interval))) H2K_log_warn(__VA_ARGS__); } while (0)
#define H2K_log_throttle_info(interval, ...) do { static u64_t __t = 0; if (H2K_printf_log_throttle(&__t, (interval))) H2K_log_info(__VA_ARGS__); } while (0)
#define H2K_log_throttle_dbg(interval, ...)  do { static u64_t __t = 0; if (H2K_printf_log_throttle(&__t, (interval))) H2K_log_dbg(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht(interval, ...)      do { static u64_t __t[MAX_HTHREADS] = {0}; if (H2K_printf_log_throttle_ht(__t, (interval))) H2K_log(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht_err(interval, ...)  do { static u64_t __t[MAX_HTHREADS] = {0}; if (H2K_printf_log_throttle_ht(__t, (interval))) H2K_log_err(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht_warn(interval, ...) do { static u64_t __t[MAX_HTHREADS] = {0}; if (H2K_printf_log_throttle_ht(__t, (interval))) H2K_log_warn(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht_info(interval, ...) do { static u64_t __t[MAX_HTHREADS] = {0}; if (H2K_printf_log_throttle_ht(__t, (interval))) H2K_log_info(__VA_ARGS__); } while (0)
#define H2K_log_throttle_ht_dbg(interval, ...)  do { static u64_t __t[MAX_HTHREADS] = {0}; if (H2K_printf_log_throttle_ht(__t, (interval))) H2K_log_dbg(__VA_ARGS__); } while (0)

#define H2K_log_string_once(S)      do { static int __f = 0; if (H2K_printf_log_once(&__f)) H2K_log_string(S); } while (0)
#define H2K_log_string_once_err(S)  do { static int __f = 0; if (H2K_printf_log_once(&__f)) H2K_log_string_err(S); } while (0)
#define H2K_log_string_once_warn(S) do { static int __f = 0; if (H2K_printf_log_once(&__f)) H2K_log_string_warn(S); } while (0)
#define H2K_log_string_once_info(S) do { static int __f = 0; if (H2K_printf_log_once(&__f)) H2K_log_string_info(S); } while (0)
#define H2K_log_string_once_dbg(S)  do { static int __f = 0; if (H2K_printf_log_once(&__f)) H2K_log_string_dbg(S); } while (0)
#define H2K_log_string_once_ht(S)      do { static u32_t __f[MAX_HTHREADS] = {0}; if (H2K_printf_log_once_ht(__f)) H2K_log_string(S); } while (0)
#define H2K_log_string_once_ht_err(S)  do { static u32_t __f[MAX_HTHREADS] = {0}; if (H2K_printf_log_once_ht(__f)) H2K_log_string_err(S); } while (0)
#define H2K_log_string_once_ht_warn(S) do { static u32_t __f[MAX_HTHREADS] = {0}; if (H2K_printf_log_once_ht(__f)) H2K_log_string_warn(S); } while (0)
#define H2K_log_string_once_ht_info(S) do { static u32_t __f[MAX_HTHREADS] = {0}; if (H2K_printf_log_once_ht(__f)) H2K_log_string_info(S); } while (0)
#define H2K_log_string_once_ht_dbg(S)  do { static u32_t __f[MAX_HTHREADS] = {0}; if (H2K_printf_log_once_ht(__f)) H2K_log_string_dbg(S); } while (0)
#define H2K_log_string_throttle(interval, S)      do { static u64_t __t = 0; if (H2K_printf_log_throttle(&__t, (interval))) H2K_log_string(S); } while (0)
#define H2K_log_string_throttle_err(interval, S)  do { static u64_t __t = 0; if (H2K_printf_log_throttle(&__t, (interval))) H2K_log_string_err(S); } while (0)
#define H2K_log_string_throttle_warn(interval, S) do { static u64_t __t = 0; if (H2K_printf_log_throttle(&__t, (interval))) H2K_log_string_warn(S); } while (0)
#define H2K_log_string_throttle_info(interval, S) do { static u64_t __t = 0; if (H2K_printf_log_throttle(&__t, (interval))) H2K_log_string_info(S); } while (0)
#define H2K_log_string_throttle_dbg(interval, S)  do { static u64_t __t = 0; if (H2K_printf_log_throttle(&__t, (interval))) H2K_log_string_dbg(S); } while (0)
#define H2K_log_string_throttle_ht(interval, S)      do { static u64_t __t[MAX_HTHREADS] = {0}; if (H2K_printf_log_throttle_ht(__t, (interval))) H2K_log_string(S); } while (0)
#define H2K_log_string_throttle_ht_err(interval, S)  do { static u64_t __t[MAX_HTHREADS] = {0}; if (H2K_printf_log_throttle_ht(__t, (interval))) H2K_log_string_err(S); } while (0)
#define H2K_log_string_throttle_ht_warn(interval, S) do { static u64_t __t[MAX_HTHREADS] = {0}; if (H2K_printf_log_throttle_ht(__t, (interval))) H2K_log_string_warn(S); } while (0)
#define H2K_log_string_throttle_ht_info(interval, S) do { static u64_t __t[MAX_HTHREADS] = {0}; if (H2K_printf_log_throttle_ht(__t, (interval))) H2K_log_string_info(S); } while (0)
#define H2K_log_string_throttle_ht_dbg(interval, S)  do { static u64_t __t[MAX_HTHREADS] = {0}; if (H2K_printf_log_throttle_ht(__t, (interval))) H2K_log_string_dbg(S); } while (0)
#else
#define H2K_log_err(...)
#define H2K_log_warn(...)
#define H2K_log_info(...)
#define H2K_log_dbg(...)
#define H2K_log(...)
#define H2K_log_string(S)
#define H2K_log_string_at(level, S)
#define H2K_log_string_err(S)
#define H2K_log_string_warn(S)
#define H2K_log_string_info(S)
#define H2K_log_string_dbg(S)
#define H2K_log_once(...)
#define H2K_log_once_err(...)
#define H2K_log_once_warn(...)
#define H2K_log_once_info(...)
#define H2K_log_once_dbg(...)
#define H2K_log_once_ht(...)
#define H2K_log_once_ht_err(...)
#define H2K_log_once_ht_warn(...)
#define H2K_log_once_ht_info(...)
#define H2K_log_once_ht_dbg(...)
#define H2K_log_throttle(interval, ...)
#define H2K_log_throttle_err(interval, ...)
#define H2K_log_throttle_warn(interval, ...)
#define H2K_log_throttle_info(interval, ...)
#define H2K_log_throttle_dbg(interval, ...)
#define H2K_log_throttle_ht(interval, ...)
#define H2K_log_throttle_ht_err(interval, ...)
#define H2K_log_throttle_ht_warn(interval, ...)
#define H2K_log_throttle_ht_info(interval, ...)
#define H2K_log_throttle_ht_dbg(interval, ...)
#define H2K_log_string_once(S)
#define H2K_log_string_once_err(S)
#define H2K_log_string_once_warn(S)
#define H2K_log_string_once_info(S)
#define H2K_log_string_once_dbg(S)
#define H2K_log_string_once_ht(S)
#define H2K_log_string_once_ht_err(S)
#define H2K_log_string_once_ht_warn(S)
#define H2K_log_string_once_ht_info(S)
#define H2K_log_string_once_ht_dbg(S)
#define H2K_log_string_throttle(interval, S)
#define H2K_log_string_throttle_err(interval, S)
#define H2K_log_string_throttle_warn(interval, S)
#define H2K_log_string_throttle_info(interval, S)
#define H2K_log_string_throttle_dbg(interval, S)
#define H2K_log_string_throttle_ht(interval, S)
#define H2K_log_string_throttle_ht_err(interval, S)
#define H2K_log_string_throttle_ht_warn(interval, S)
#define H2K_log_string_throttle_ht_info(interval, S)
#define H2K_log_string_throttle_ht_dbg(interval, S)
#endif
#endif

#endif
