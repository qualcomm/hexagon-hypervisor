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

#ifdef H2K_LOGBUF

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

static u32_t logbuf_space() {

	return LOGBUF_SIZE - H2K_gp->logbuf_pos - 1;
}

static void logbuf_wrap() {

	H2K_gp->logbuf_pos = 0;
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

char *H2K_logbuf_alloc(u32_t count) {

	char *ret;

	if (count > logbuf_space()) {
		logbuf_wrap();
	}
	ret = H2K_gp->logbuf + H2K_gp->logbuf_pos;
	H2K_gp->logbuf_pos += count;

	return ret;
}

u32_t H2K_do_log_string(const char *string) {

	u32_t len = strlen(string);
	s32_t ret = len;

	H2K_spinlock_lock(&H2K_gp->logbuf_lock);

	if (H2K_gp->logbuf_enable) {
		strcpy(H2K_logbuf_alloc(len + 1), string);
	}

	if (H2K_gp->log_enable) {
		ret = H2K_write(1, (const u8_t *)"H2K: ", 5);
		if (ret < 0) goto out;

		ret = H2K_write(1, (const u8_t *)string, len);
	}

 out:
	H2K_spinlock_unlock(&H2K_gp->logbuf_lock);
	return ret;
}

/* Call with logbuf_lock held */
static void emit_char(char c) {
	char *logbuf_ptr = H2K_logbuf_alloc(1);

	*logbuf_ptr = c;
	*(logbuf_ptr + 1) =  '\0';  // next char will overwrite
}

static u32_t emit_string(const char *s) {
	u32_t len = 0;

	while ('\0' != *s) {
		emit_char(*s++);
		len++;
	}
	return len;
}

#define BUF_SIZE 32
u32_t num(u64_t n, u32_t base, u32_t width, u32_t neg) {
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
	emit_string((const char *)p);
	return len;
}

s32_t H2K_log_print(const char *fmt, ...) {

	H2K_spinlock_lock(&H2K_gp->logbuf_lock);

	va_list args;
	u32_t start = H2K_gp->logbuf_pos;
	u32_t width;
	s64_t dec;
	s32_t ret;
	u32_t len = 0;
	u32_t parsing = 0;
	u32_t longlong = 0;

	/* if either is enabled we need logbuf to hold the output */
	if (!H2K_gp->logbuf_enable && !H2K_gp->log_enable) {
		H2K_spinlock_unlock(&H2K_gp->logbuf_lock);
		return 0;
	}

	va_start(args, fmt);

	while (*fmt != '\0') {
		if (!parsing && '%' != *fmt) {
			emit_char(*fmt);
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
				emit_char(*fmt);
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
					len += num((u64_t)va_arg(args, u64_t), 16, width, 0);
				} else {
					len += num((u64_t)va_arg(args, u32_t), 16, width, 0);
				}
				break;

			case 'd':
				if (longlong) {
					dec = (s64_t)va_arg(args, s64_t);
				} else {
					dec = (s64_t)va_arg(args, s32_t);
				}
				if (0 > dec) {
					len += num((u64_t)(-dec), 10, width, 1);
				} else {
					len += num((u64_t)dec, 10, width, 0);
				}
				break;
			case 's':
				len += emit_string((const char *)va_arg(args, char *));
				break;

			default:
				break;
			}
		}
		if (!parsing) fmt++;
	}

	va_end(args);

	ret = len;
	if (H2K_gp->log_enable) {
		ret = H2K_write(1, (const u8_t *)"H2K: ", 5);
		if (ret < 0) goto out;

		if (H2K_gp->logbuf_pos < start) {  // wrapped
			ret = H2K_write(1, (const u8_t *)(H2K_gp->logbuf + start), LOGBUF_SIZE - start - 1);
			if (ret < 0) goto out;
			ret = H2K_write(1, (const u8_t *)(H2K_gp->logbuf), len - (LOGBUF_SIZE - start - 1));
			if (ret < 0) goto out;
		} else {
			ret = H2K_write(1, (const u8_t *)(H2K_gp->logbuf + start), len);
			if (ret < 0) goto out;
		}
	}

 out:
	H2K_spinlock_unlock(&H2K_gp->logbuf_lock);
	return ret;
}

char *H2K_log_init() {

	if (NULL == (H2K_gp->logbuf = (char *)H2K_mem_alloc(LOGBUF_SIZE))) {
		return NULL;
	}
	H2K_bzero(H2K_gp->logbuf, LOGBUF_SIZE);
	H2K_gp->logbuf_pos = 0;
	H2K_gp->logbuf_enable = 1;
	H2K_gp->log_enable = 1;

	H2K_spinlock_init(&H2K_gp->logbuf_lock);

	return H2K_gp->logbuf;
}

#endif
