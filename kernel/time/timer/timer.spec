:mod:`timer` -- Timer hardware and timeout services
===================================================

.. module:: timer

H2K_timer_init
--------------

.. c:function:: void H2K_timer_init(u32_t devpage_offset)

	:param devpage_offset: Subsystem device-page offset

Initializes the timer device, timeout tree, timer registers, and timer
interrupt when devices are enabled.


H2K_timer_int
-------------

.. c:function:: void H2K_timer_int(u32_t intnum, H2K_thread_context *me, u32_t hwtnum)

	:param intnum: Timer interrupt number
	:param me: Interrupted thread
	:param hwtnum: Hardware-thread number

Reads the current timer, expires all due thread timeouts, programs the next
expiry, and re-enables the timer interrupt.


H2K_timer_cancel_withlock
-------------------------

.. c:function:: static inline void H2K_timer_cancel_withlock(H2K_thread_context *me)

	:param me: Thread whose timeout is cancelled

Removes the thread's timeout from the timeout tree.  The caller must hold the
BKL.


H2K_timer_cancel
----------------

.. c:function:: static inline void H2K_timer_cancel(H2K_thread_context *me)

	:param me: Thread whose timeout is cancelled

Acquires the BKL, calls :c:func:`H2K_timer_cancel_withlock()`, and releases the
BKL.


H2K_timer_trap
--------------

.. c:function:: u64_t H2K_timer_trap(u32_t traptype, u64_t arg, H2K_thread_context *me)

	:param traptype: Timer operation
	:param arg: Operation argument
	:param me: Current thread
	:returns: Operation result, or ``~0ULL`` for an invalid operation

Dispatches timer operations for frequency, resolution, current time, timeout
query, absolute timeout, and relative timeout.


H2K_vmtrap_timer
----------------

.. c:function:: void H2K_vmtrap_timer(H2K_thread_context *me)

	:param me: Current thread

Passes ``me->r00`` and ``me->r0302`` to :c:func:`H2K_timer_trap()` and stores
the result in ``me->r0100``.


Timer operations
----------------

The timer trap handlers provide these operations:

* frequency: exact timer nanoseconds-per-second value;
* resolution: nanoseconds per hardware tick;
* time: current time since boot in nanoseconds;
* timeout: current thread's active timeout, or zero;
* set timeout: set an absolute nanosecond deadline;
* delta timeout: set a relative timeout.

Invalid or expired deadlines deactivate the timeout.  The timeout tree is
updated under the BKL, and the hardware timer is reprogrammed when the new
deadline is earlier than the current programmed deadline.
