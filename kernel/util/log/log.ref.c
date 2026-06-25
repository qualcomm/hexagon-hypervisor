/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <spinlock.h>
#include <alloc.h>
#include <bzero.h>
#include <max.h>
#include <globals.h>
#include <cache.h>
#include <angel.h>
#include <hw.h>
#include <atomic.h>
#include <log.h>

#ifdef H2K_LOGBUF

/* log_level lives here rather than in H2K_gp to avoid perturbing the global
 * struct layout in no-logging builds. */
#ifndef H2K_LOG_LEVEL
#define H2K_LOG_LEVEL H2K_LOG_DBG
#endif
static u32_t log_level = H2K_LOG_LEVEL;

static u32_t strcpy(char *dest, const char *src)
{
	u32_t count = 1;

	while ('\0' != (*dest++ = *src++)) {
		count++;
	}

	return count;
}

static u32_t strlen(const char *src)
{
	u32_t count = 0;

	while ('\0' != (*src++)) {
		count++;
	}

	return count;
}

static u32_t logbuf_space(u32_t htid) {

	return LOGBUF_PER_HT_SIZE - H2K_gp->logbuf_pos[htid] - 1;
}

static void logbuf_wrap(u32_t htid) {

	H2K_gp->logbuf_pos[htid] = 0;
}

s32_t H2K_write(u32_t fd, const u8_t *buf, u32_t len) {
	struct req { u32_t fd; u8_t *buf; u32_t c; } x;
	s32_t ret;
	u32_t bytes_left = len;  // left to write
	u32_t bytes_done = 0;

	while (bytes_left > 0) {

		x.fd = fd;
		x.buf = ((u8_t *)buf) - H2K_gp->phys_offset + bytes_done;
		x.c = bytes_left;
		H2K_dccleana_range((void *)buf + bytes_done, bytes_left);
		H2K_dccleana_range((void *)&x, sizeof(struct req));

		ret = H2K_trap_angel(SYS_WRITE, (void *)(((u32_t)&x) - H2K_gp->phys_offset), 0);

		if (ret < 0) {  // error
			return ret;
		}
		bytes_left = ret;  // angel transactor apparently gets this backwards from write()
		bytes_done = len - bytes_left;
	}
	return len;
}

/*
 * Allocate count bytes in the calling hthread's own ring.  Lockless: only the
 * owning hthread (identified by htid) ever writes its ring.
 */
char *H2K_logbuf_alloc(u32_t htid, u32_t count) {

	char *ret;

	if (count > logbuf_space(htid)) {
		logbuf_wrap(htid);
	}
	ret = H2K_gp->logbuf[htid] + H2K_gp->logbuf_pos[htid];
	H2K_gp->logbuf_pos[htid] += count;

	return ret;
}

u32_t H2K_do_log_string(const char *string) {

	u32_t htid = get_hwtnum();
	u32_t len = strlen(string);
	s32_t ret = len;

	if (H2K_gp->logbuf_enable) {
		strcpy(H2K_logbuf_alloc(htid, len + 1), string);
	}

	if (H2K_gp->log_enable) {
		/* the angel SYS_WRITE channel is shared hardware: serialize it */
		H2K_spinlock_lock(&H2K_gp->logbuf_lock);
		ret = H2K_write(1, (const u8_t *)"H2K: ", 5);
		if (ret >= 0) {
			ret = H2K_write(1, (const u8_t *)string, len);
		}
		H2K_spinlock_unlock(&H2K_gp->logbuf_lock);
	}

	return ret;
}

/* Append one char to htid's ring, keeping a trailing NUL for string reads. */
static void emit_char(u32_t htid, char c) {
	char *logbuf_ptr = H2K_logbuf_alloc(htid, 1);

	*logbuf_ptr = c;
	*(logbuf_ptr + 1) =  '\0';  // next char will overwrite
}

static u32_t emit_string(u32_t htid, const char *s) {
	u32_t len = 0;

	while ('\0' != *s) {
		emit_char(htid, *s++);
		len++;
	}
	return len;
}

#define BUF_SIZE 32
u32_t num(u32_t htid, u64_t n, u32_t base, u32_t width, u32_t neg) {
	char buf[BUF_SIZE];
	char *p = buf + BUF_SIZE;
	u32_t len = 0;
	u32_t zeros = 0;

	*(--p) = '\0';

	do {
		*(--p) = ((n % base) >= 10 ? 'a' + (u8_t)(n % base) - 10 : '0' + (u8_t)(n % base));
		n /= base;
		len++;
	 } while (0 < n);

	if (0 < width) {
		zeros = width - len;
		while (0 < zeros) {
			*(--p) = '0';
			zeros--;
			len++;
		}
	}
	if (neg) {
		*(--p) = '-';
		len++;
	}
	emit_string(htid, (const char *)p);
	return len;
}

/* Indexed by H2K_LOG_* severity level. */
static const char level_char[] = { 'E', 'W', 'I', 'D' };

/*
 * Emit prefix into htid's ring and return its length:
 *   [ht<id>][<sev>][<pcycle>][<file>:<line>]<space>
 */
static u32_t emit_prefix(u32_t htid, u32_t level, const char *file, u32_t line) {
	const char *base = file;
	const char *p = file;
	u32_t len = 0;
	char sev = (level < sizeof(level_char)) ? level_char[level] : '?';

	/* keep the basename only, dropping any leading directory path */
	while ('\0' != *p) {
		if ('/' == *p) base = p + 1;
		p++;
	}

	len += emit_string(htid, "[ht");
	len += num(htid, (u64_t)htid, 10, 0, 0);
	len += emit_string(htid, "][");
	emit_char(htid, sev); len++;
	len += emit_string(htid, "][");
	len += num(htid, H2K_get_pcycle_reg(), 10, 0, 0);
	len += emit_string(htid, "][");
	len += emit_string(htid, base);
	emit_char(htid, ':'); len++;
	len += num(htid, (u64_t)line, 10, 0, 0);
	len += emit_string(htid, "] ");

	return len;
}

s32_t H2K_log_print(u32_t level, const char *file, u32_t line, const char *fmt, ...) {

	u32_t htid = get_hwtnum();
	va_list args;
	char *buf;
	u32_t start;
	u32_t width;
	s64_t dec;
	s32_t ret;
	u32_t len = 0;
	u32_t parsing = 0;
	u32_t longlong = 0;

	/* if either is enabled we need logbuf to hold the output */
	if (!H2K_gp->logbuf_enable && !H2K_gp->log_enable) {
		return 0;
	}

	/* drop messages below the active runtime verbosity threshold */
	if (level > log_level) {
		return 0;
	}

	/* lockless: format directly into this hthread's own ring */
	buf = H2K_gp->logbuf[htid];
	start = H2K_gp->logbuf_pos[htid];

	len += emit_prefix(htid, level, file, line);

	va_start(args, fmt);

	while (*fmt != '\0') {
		if (!parsing && '%' != *fmt) {
			emit_char(htid, *fmt);
			len++;
		} else {
			if (!parsing) {
				fmt++;  // skip %
				width = 0;
				longlong = 0;
			}
			parsing = 0;

			switch(*fmt) {
			case '\0':
				break;

			case '%':
				emit_char(htid, *fmt);
				len++;
				break;

			case '0':
				while ('0' == *fmt) fmt++;
				while ('0' <= *fmt && *fmt <= '9') {
					width *= 10;
					width += *fmt - '0';
					fmt++;
				}
				parsing = 1;
				break;
			case 'l':
				fmt++;
				if ('l' == *fmt) {
					longlong = 1;
					fmt++;
				}
				parsing = 1;
				break;
			case 'x':
				if (longlong) {
					len += num(htid, (u64_t)va_arg(args, u64_t), 16, width, 0);
				} else {
					len += num(htid, (u64_t)va_arg(args, u32_t), 16, width, 0);
				}
				break;

			case 'd':
				if (longlong) {
					dec = (s64_t)va_arg(args, s64_t);
				} else {
					dec = (s64_t)va_arg(args, s32_t);
				}
				if (0 > dec) {
					len += num(htid, (u64_t)(-dec), 10, width, 1);
				} else {
					len += num(htid, (u64_t)dec, 10, width, 0);
				}
				break;
			case 's':
				len += emit_string(htid, (const char *)va_arg(args, char *));
				break;

			default:
				break;
			}
		}
		if (!parsing) fmt++;
	}

	va_end(args);

	ret = len;
	/* optional live streaming: serialize the shared angel channel */
	if (H2K_gp->log_enable) {
		H2K_spinlock_lock(&H2K_gp->logbuf_lock);
		if (H2K_gp->logbuf_pos[htid] < start) {  // wrapped mid-message
			ret = H2K_write(1, (const u8_t *)(buf + start), LOGBUF_PER_HT_SIZE - start - 1);
			if (ret >= 0) {
				ret = H2K_write(1, (const u8_t *)buf, len - (LOGBUF_PER_HT_SIZE - start - 1));
			}
		} else {
			ret = H2K_write(1, (const u8_t *)(buf + start), len);
		}
		H2K_spinlock_unlock(&H2K_gp->logbuf_lock);
	}

	return ret;
}

/*
 * once/throttle gating.  State lives in a static local at each callsite
 * (via the macros in log.h) and is passed in by pointer.
 * Each function returns 1 when the guarded message should emit.
 */

/* GLOBAL once: atomic test-and-set on the callsite flag.
 * H2K_atomic_setbit returns 1 when the bit was clear (first setter wins). */
u32_t H2K_log_once(u32_t *flag) {
	return H2K_atomic_setbit(flag, 0);
}

/* PER-HT once: caller passes flag[MAX_HTHREADS]; only this ht touches its
 * slot so a plain test+set is race-free. */
u32_t H2K_log_once_ht(u32_t *flags) {
	u32_t htid = get_hwtnum();
	if (flags[htid]) return 0;
	flags[htid] = 1;
	return 1;
}

/* GLOBAL throttle: caller passes a shared last-fired timestamp.
 * Serialized with logbuf_lock since two hthreads may race on it. */
u32_t H2K_log_throttle(u64_t *last, u64_t interval) {
	u64_t now = H2K_get_pcycle_reg();
	u32_t go = 0;

	H2K_spinlock_lock(&H2K_gp->logbuf_lock);
	if (now - *last >= interval) {
		*last = now;
		go = 1;
	}
	H2K_spinlock_unlock(&H2K_gp->logbuf_lock);
	return go;
}

/* PER-HT throttle: caller passes last[MAX_HTHREADS]; only this ht touches
 * its slot so no lock is needed. */
u32_t H2K_log_throttle_ht(u64_t *last, u64_t interval) {
	u32_t htid = get_hwtnum();
	u64_t now = H2K_get_pcycle_reg();

	if (now - last[htid] < interval) return 0;
	last[htid] = now;
	return 1;
}

char *H2K_log_init() {
	u32_t htid;

	for (htid = 0; htid < MAX_HTHREADS; htid++) {
		if (NULL == (H2K_gp->logbuf[htid] = (char *)H2K_mem_alloc(LOGBUF_PER_HT_SIZE))) {
			return NULL;
		}
		H2K_bzero(H2K_gp->logbuf[htid], LOGBUF_PER_HT_SIZE);
		H2K_gp->logbuf_pos[htid] = 0;
	}

	H2K_gp->logbuf_enable = 1;
	H2K_gp->log_enable = 1;
	log_level = H2K_LOG_LEVEL;

	H2K_spinlock_init(&H2K_gp->logbuf_lock);

	return H2K_gp->logbuf[0];
}

#endif
