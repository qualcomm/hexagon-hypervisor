/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

/*
 * libs/virtio/test_h2_mt -- multi-threaded console stress test.
 *
 * VC_MT_THREADS threads print VC_MT_LINES lines each, simultaneously, via
 * atomic_printf() (snprintf() into a stack buffer, then a single write()).
 * The point is to prove the console write path is atomic under
 * concurrency: with 8 producers sharing ONE bounce buffer and ONE transmit
 * queue (virtio) or ONE semihosting trap plus the ANGEL mirror ring (angel),
 * nothing may be lost, duplicated, reordered (per thread) or byte-corrupted.
 * validate_log.py re-parses the captured log and enforces that.
 *
 * The stress ALWAYS runs; only the transport that carries the write()
 * differs, and the run announces which one it used so the log is
 * self-describing:
 *
 *   transport=virtio-console  write -> sys_write_internal -> virtio_console_write
 *                             (built with VIRTIO_CONSOLE and this VM owns or was
 *                             granted a console port)
 *   transport=angel           write -> sys_write_internal -> ANGEL semihosting
 *                             (built without VIRTIO_CONSOLE, or the device is
 *                             not available to this VM)
 *
 * Both transports are worth stressing -- they are two different locks on the
 * same write() path -- so neither case is skipped.  The transport is reported
 * twice: once human-readably ("VC_MT_TRANSPORT ... reason=...") and once as
 * transport=<name> on the machine-parsed VC_MT_CONFIG line, per stream, so a
 * merged multi-VM log records the transport of each VM separately.
 *
 * Each line is self-describing so the validator needs no external state:
 *
 *     V<stream> T<tid> S<seq> N<len> C<crc32> <payload>\n
 *
 *   stream  : guest-supplied stream id (see argv below), identifies WHICH VM
 *             produced the line when several instances of this test run at
 *             once and their console output lands in one merged log
 *   tid     : thread id            (0 .. THREADS-1), local to this VM/stream
 *   seq     : per-thread sequence  (0 .. LINES-1), strictly increasing per tid
 *   len     : payload byte count
 *   crc32   : reflected CRC-32 (poly 0xEDB88320, == Python zlib.crc32) of payload
 *   payload : len bytes from the hex alphabet [0-9a-f] -- chosen so a line body
 *             can never spell ERROR/FAIL/PASSED/status and trip the results.txt
 *             grep, while any byte swap/drop/dup still changes the CRC.
 *
 * Each line is emitted with a SINGLE atomic_printf() call -- format into a
 * stack buffer, then ONE write() of the whole block -- so one log line == one
 * console write call: line atomicity is then purely a function of the write
 * path's internal lock (what we are testing).
 *
 * Guest command line (forwarded by the booter into argv, see
 * libs/h2_compat/plainmutex/h2_test/stress_lock/test.c for the mechanism):
 *
 *     test.elf [stream_id [seed]]
 *
 *   stream_id : distinguishes this VM's output from every other VM's when
 *               many instances run simultaneously against a shared/merged
 *               console log (default 0, so a single standalone run needs no
 *               argument).  validate_log.py keys everything off (stream, tid,
 *               seq) instead of just (tid, seq) so it can validate all
 *               streams from all threads from all VMs out of one log.
 *   seed      : optional; overrides the RNG seed (see below) so a failing
 *               run can be replayed with "make GUEST_ARGS=\"<stream_id>
 *               0x<seed>\"".
 *
 * Reproducibility: the RNG seed is printed as "VC_MT_SEED=0x..." before any
 * thread starts.  Passing it back as the 2nd cmdline argument (or rebuilding
 * with -DVC_MT_SEED=<value> when no cmdline seed is given) replays every
 * thread's delay sequence.  (Wall-clock interleaving on real hardware is not
 * bit-exact, but the validator is order-independent across threads, so a pass
 * is deterministic regardless of interleaving.)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <h2.h>
#include <pthread.h>
#include <h2_cycles.h>          /* h2_get_pcycles() for the RNG seed */

#ifndef STDOUT_FILENO
#define STDOUT_FILENO 1         /* not defined by this toolchain's unistd.h */
#endif

/*
 * Standalone-on-master build: the virtio-console library doesn't exist on
 * this branch, so the setup/ready() calls below are commented out rather
 * than left behind an #ifdef VIRTIO_CONSOLE -- this test always runs the
 * ANGEL transport here.
 *
#ifdef VIRTIO_CONSOLE
#include <virtio_console.h>
#ifndef VIRTIO_CONSOLE_ADDR
#define VIRTIO_CONSOLE_ADDR 0x11000000
#endif
#ifndef VIRTIO_WINDOW
#define VIRTIO_WINDOW 0x1000000
#endif
#endif
 */

/* Transport names reported in the log and matched by validate_log.py. */
#define XPORT_VIRTIO "virtio-console"
#define XPORT_ANGEL  "angel"

#ifndef VC_MT_THREADS
#define VC_MT_THREADS 8u
#endif
#ifndef VC_MT_LINES
/*
 * Lines printed per thread.  Over virtio each line is one synchronous console
 * round-trip (post + poll the used ring) and all threads serialize through the
 * single TX queue, so wall-clock is dominated by the archsim device latency
 * (~tens of ms per write); over ANGEL each line is one semihosting trap and is
 * much cheaper.  The default is sized to exercise heavy concurrency (ring wrap,
 * mirror-ring wrap, all corner-case lengths, 8-way contention) while finishing
 * in a couple of minutes on the slower (virtio) transport.  Raise it for a soak
 * run, e.g. -DVC_MT_LINES=10000 (that is ~80k serialized device round-trips and
 * takes hours in the simulator).
 */
#define VC_MT_LINES 100u
#endif
#ifndef VC_MT_MAX_SPIN
#define VC_MT_MAX_SPIN 256u    /* upper bound on the inter-print nop delay */
#endif

/* Line payload lengths cycled by sequence number.  Includes values around and
 * above VC_BOUNCE_SZ (512) so the driver's multi-chunk path is exercised and
 * proven to stay atomic (the lock must span all chunks of one write). */
static const unsigned int len_tab[] = {
	1u, 7u, 63u, 200u, 480u, 511u, 512u, 513u, 600u, 700u
};
#define LEN_TAB_N (sizeof(len_tab) / sizeof(len_tab[0]))

/* snprintf() into a stack buffer then write() the whole block in one call, so
 * a line can never be split by a concurrent write on the same fd. */
static void atomic_printf(const char *fmt, ...)
{
	char buf[1024];
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (len < 0)
		return;
	if ((unsigned int)len >= sizeof(buf))
		len = sizeof(buf) - 1;

	write(STDOUT_FILENO, buf, (size_t)len);
}

static h2_sem_t go;            /* released once, after all threads are parked */
static h2_sem_t started;       /* each thread posts when parked on `go`       */

/* Set once in main() before any thread is created; read-only afterwards, so
 * workers need no synchronization to read it. */
static unsigned int g_stream_id;

static void FAIL(const char *msg)
{
	atomic_printf("FAIL: %s\n", msg);
	exit(1);
}

/* Reflected CRC-32, poly 0xEDB88320, init/final 0xFFFFFFFF -- matches
 * Python's zlib.crc32() exactly. */
static unsigned int crc32_calc(const char *p, unsigned int n)
{
	unsigned int crc = 0xFFFFFFFFu;
	unsigned int i;
	int k;
	for (i = 0; i < n; i++) {
		crc ^= (unsigned char)p[i];
		for (k = 0; k < 8; k++) {
			unsigned int mask = -(crc & 1u);
			crc = (crc >> 1) ^ (0xEDB88320u & mask);
		}
	}
	return crc ^ 0xFFFFFFFFu;
}

/* xorshift32 -- deterministic per-thread delay stream. */
static inline unsigned int xrand(unsigned int *s)
{
	unsigned int x = *s;
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	*s = x ? x : 0xA5A5A5A5u;   /* never latch to zero */
	return *s;
}

struct worker_arg {
	unsigned int tid;
	unsigned int seed;
};

static struct worker_arg args[VC_MT_THREADS];

static void *worker(void *p)
{
	struct worker_arg *a = (struct worker_arg *)p;
	unsigned int tid   = a->tid;
	unsigned int state = a->seed;
	unsigned int seq;

	/* Park until every thread is ready, then all are released together so
	 * contention on the shared console peaks immediately. */
	h2_sem_up(&started);
	h2_sem_down(&go);

	for (seq = 0; seq < VC_MT_LINES; seq++) {
		unsigned int len = len_tab[seq % LEN_TAB_N];
		char payload[701];
		unsigned int crc, i, d;

		for (i = 0; i < len; i++) {
			unsigned int h = (tid * 7u + seq * 13u + i) & 0xFu;
			payload[i] = "0123456789abcdef"[h];
		}
		crc = crc32_calc(payload, len);

		/* Small random delay (occasionally zero -> burst contention). */
		d = xrand(&state) % VC_MT_MAX_SPIN;
		for (i = 0; i < d; i++) {
			asm volatile("nop");
		}

		atomic_printf("V%u T%u S%u N%u C%08x %.*s\n",
		       g_stream_id, tid, seq, len, crc, (int)len, payload);
	}
	return NULL;
}

/*
 * Bring up the virtio-console for this VM when the build has one, and announce
 * in the log which transport will carry this run's output.  Returns the
 * transport name for the VC_MT_CONFIG line.  Never fails: an unavailable
 * virtio-console just means atomic_printf() keeps going straight to ANGEL,
 * which is stressed exactly the same way.
 */
static const char *announce_transport(unsigned int stream_id)
{
	/*
	 * Standalone-on-master build: no virtio-console library here, so the
	 * setup call is commented out and this test always reports/uses ANGEL.
	 *
	 * int rc, ready;
	 *
	 * rc = virtio_console_setup(VIRTIO_CONSOLE_ADDR, VIRTIO_WINDOW);
	 * ready = virtio_console_ready();
	 * if (0 == rc && ready) {
	 *	atomic_printf("VC_MT_TRANSPORT stream=%u transport=" XPORT_VIRTIO
	 *	       " reason=virtio-console ready; test output is directed to "
	 *	       "the virtio-console\n", stream_id);
	 *	return XPORT_VIRTIO;
	 * }
	 */
	atomic_printf("VC_MT_TRANSPORT stream=%u transport=" XPORT_ANGEL
	       " reason=built without VIRTIO_CONSOLE; test output is directed to "
	       "ANGEL instead of virtio-console\n", stream_id);
	return XPORT_ANGEL;
}

int main(int argc, char **argv)
{
	pthread_t th[VC_MT_THREADS];
	unsigned long long seed;
	const char *xport;
	unsigned int stream_id;
	unsigned int i;

	h2_handle_errors(1);

	/* argv[1] = stream id (default 0, so a standalone single-VM run needs no
	 * argument); argv[2] = optional seed override.  See the file header
	 * comment for the full guest cmdline contract. */
	stream_id = (argc > 1) ? (unsigned int)strtoul(argv[1], NULL, 0) : 0u;
	if (argc > 2) {
		seed = strtoull(argv[2], NULL, 0);
	} else {
#ifdef VC_MT_SEED
		seed = (unsigned long long)(VC_MT_SEED);
#else
		seed = h2_get_pcycles();
#endif
	}
	g_stream_id = stream_id;

	/* Select + report the output transport before anything else is logged. */
	xport = announce_transport(stream_id);

	/* Announce config + seed before spawning. */
	atomic_printf("VC_MT_SEED=0x%llx\n", seed);
	atomic_printf("VC_MT_CONFIG stream=%u threads=%u lines=%u transport=%s\n",
	       stream_id, (unsigned)VC_MT_THREADS, (unsigned)VC_MT_LINES, xport);

	h2_sem_init_val(&go, 0);
	h2_sem_init_val(&started, 0);

	for (i = 0; i < VC_MT_THREADS; i++) {
		args[i].tid  = i;
		/* Distinct, seed-derived per-thread delay sequence. */
		args[i].seed = (unsigned int)(seed ^ ((unsigned long long)(i + 1u)
		                              * 0x9E3779B1ull));
		if (args[i].seed == 0) {
			args[i].seed = 0xA5A5A5A5u;
		}
		if (pthread_create(&th[i], NULL, worker, &args[i]) != 0) {
			FAIL("pthread_create");
		}
	}

	/* Wait until all threads are parked, then release them together. */
	for (i = 0; i < VC_MT_THREADS; i++) {
		h2_sem_down(&started);
	}
	for (i = 0; i < VC_MT_THREADS; i++) {
		h2_sem_up(&go);
	}

	for (i = 0; i < VC_MT_THREADS; i++) {
		if (pthread_join(th[i], NULL) != 0) {
			FAIL("pthread_join");
		}
	}

	/* Completion marker only -- validate_log.py decides PASS/FAIL by
	 * re-parsing the full log (per-thread completeness + CRC integrity). */
	atomic_printf("VC_MT COMPLETE stream=%u\n", stream_id);
	exit(0);
}
