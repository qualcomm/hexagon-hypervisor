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

.. c:function:: void H2K_timer_int(u32_t unused, H2K_thread_context *me, u32_t hwtnum)

	:param unused: Unused parameter
	:param me: Interrupted thread
	:param hwtnum: Hardware-thread number

Reads the current timer, expires all due thread timeouts, programs the next
expiry, and re-enables the timer interrupt.  It acquires the BKL before
``timer_lock`` to preserve the kernel lock order.


H2K_timer_cancel
----------------

.. c:function:: static inline void H2K_timer_cancel(H2K_thread_context *me)

	:param me: Thread whose timeout is cancelled

Acquires ``timer_lock``, removes the thread's timeout from the timeout tree,
and releases the lock.


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

Invalid or expired deadlines deactivate the timeout.  Timer state and the
timeout tree are updated under ``timer_lock``; the hardware timer is
reprogrammed when a new deadline is earlier than the current deadline.
