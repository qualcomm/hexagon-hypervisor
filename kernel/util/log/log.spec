:mod:`log` -- Kernel logging facility
=========================================

.. module:: log

Logging is selected at build time.  ``H2K_LOGBUF`` enables per-hardware-thread
ring buffers and optional live output; ``H2K_LOG_PRINTF`` selects a printf
backend when ``H2K_LOGBUF`` is not enabled.  With neither option, the logging
macros expand to no-ops.  ``H2K_LOG_LEVEL`` defaults to ``H2K_LOG_DBG`` and
filters messages whose level is numerically greater than the configured level.

Severity constants are ``H2K_LOG_ERR`` (0), ``H2K_LOG_WARN`` (1),
``H2K_LOG_INFO`` (2), and ``H2K_LOG_DBG`` (3).


H2K_write
---------

.. c:function:: s32_t H2K_write(u32_t fd, const u8_t *buf, u32_t len)

	:param fd: Angel output file descriptor
	:param buf: Buffer to write
	:param len: Number of bytes
	:returns: ``len`` on success, or a negative Angel error

Writes the buffer through the Angel trap, retrying until all bytes are
transferred.  Returns an error immediately if a write fails.


H2K_logbuf_alloc
----------------

.. c:function:: char *H2K_logbuf_alloc(u32_t htid, u32_t count)

	:param htid: Hardware-thread ring to use
	:param count: Number of bytes to reserve
	:returns: Start of the reserved region

Reserves ``count`` bytes in the selected hardware thread's log ring.  If the
remaining space is insufficient, wraps the position to the beginning.  The
ring is lockless because each hardware thread owns its own write position.


H2K_do_log_string_at
--------------------

.. c:function:: u32_t H2K_do_log_string_at(u32_t level, const char *file, u32_t line, const char *string)

	:returns: Number of message bytes emitted, or 0 when logging is disabled or filtered

Formats the standard ``[ht][LEVEL][pcycles][file:line]`` prefix and the supplied
string into the current hardware thread's ring.  When live output is enabled,
serializes writes through ``logbuf_lock`` and streams the message through
:c:func:`H2K_write()`.


H2K_log_print
-------------

.. c:function:: s32_t H2K_log_print(u32_t level, const char *file, u32_t line, const char *format, ...)

	:returns: Formatted message length, or the live-output write result

Formats the supported printf conversions ``%``, ``%x``, ``%d``, ``%s``, optional
zero-padding width, and ``l``/``ll`` integer modifiers.  It applies the same
enablement and severity filtering as :c:func:`H2K_do_log_string_at()`.


H2K_log_once / H2K_log_once_ht
------------------------------

.. c:function:: u32_t H2K_log_once(u32_t *flag)
.. c:function:: u32_t H2K_log_once_ht(u32_t *flags)

The global form atomically returns 1 only for the first caller that sets
``flag`` bit zero.  The per-hardware-thread form uses the current hardware
thread's element in ``flags[MAX_HTHREADS]`` and returns 1 only once for that
thread.


H2K_log_throttle / H2K_log_throttle_ht
--------------------------------------

.. c:function:: u32_t H2K_log_throttle(u64_t *last, u64_t interval)
.. c:function:: u32_t H2K_log_throttle_ht(u64_t *last, u64_t interval)

Compare the current pcycle count with the previous timestamp.  Return 1 and
update the timestamp when at least ``interval`` cycles have elapsed; otherwise
return 0.  The global form serializes access with ``logbuf_lock``.  The
per-hardware-thread form uses one timestamp per hardware thread without a lock.


H2K_log_init
------------

.. c:function:: char *H2K_log_init(void)

	:returns: The first allocated log buffer, or ``NULL`` if allocation fails

Allocates and clears one ``LOGBUF_PER_HT_SIZE`` buffer per hardware thread,
resets positions, enables buffered/live logging, loads ``H2K_LOG_LEVEL``, and
initializes the log-buffer lock.


Logging macros
--------------

When logging is enabled, ``H2K_log_err``, ``H2K_log_warn``, ``H2K_log_info``,
``H2K_log_dbg``, and ``H2K_log`` call ``H2K_log_print`` with the corresponding
severity.  The ``H2K_log_string*`` family calls ``H2K_do_log_string_at``.
Each severity has ``once``, ``once_ht``, ``throttle``, and ``throttle_ht``
variants, plus string variants.  These macros keep their gating state in
static call-site storage.  In the disabled build they evaluate to no-ops.

With ``H2K_LOG_PRINTF``, the same macro families print a prefix containing
severity, pcycles, basename, and source line; their gating uses the printf
backend helpers.


H2K_printf_log_once
-------------------

.. c:function:: u32_t H2K_printf_log_once(int *flag)

	:param flag: Call-site once flag
	:returns: 1 for the first caller, otherwise 0

The printf-backend equivalent of :c:func:`H2K_log_once()`.  It atomically
sets bit zero of the call-site flag.


H2K_printf_log_throttle
-----------------------

.. c:function:: u32_t H2K_printf_log_throttle(u64_t *last, u64_t interval)

	:param last: Shared last-emission timestamp
	:param interval: Minimum pcycle interval
	:returns: 1 when emission is allowed, otherwise 0

The printf-backend equivalent of :c:func:`H2K_log_throttle()`.  It serializes
access to the timestamp with a private spinlock and allows emission once the
specified number of cycles has elapsed.
