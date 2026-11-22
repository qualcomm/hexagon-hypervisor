:mod:`sample` -- Cross-hardware-thread profiling samples
==========================================================

.. module:: sample

The sampling path is enabled by ``DO_PROFILE``.  A sampling request is sent
through the VM IPI mechanism so each active hardware thread records its current
thread and PC.

H2K_sample
----------

.. c:function:: void H2K_sample(u32_t hthread, H2K_thread_context *me)

	:param hthread: Hardware thread receiving the sample request
	:param me: Current thread context, or NULL when the hardware thread is idle

Description
~~~~~~~~~~~

Records one profiling sample for a hardware thread.

Functionality
~~~~~~~~~~~~~

Atomically clears that hardware thread's pending bit.  Only the first handler
for the request records a result: the current thread ID and ELR are packed
into ``prof_sample[hthread]``; an idle hardware thread records zero.


H2K_sample_start
-----------------

.. c:function:: u32_t H2K_sample_start(H2K_thread_context *me)

	:param me: Current thread requesting the sample
	:returns: Mask of other active hardware threads

Description
~~~~~~~~~~~

Requests and collects one profiling sample from every other active hardware
thread.

Functionality
~~~~~~~~~~~~~

If another sample is already in progress, returns zero.  Otherwise, sets the
pending mask for all active hardware threads except the caller, sends them a
VM IPI, and waits until their handlers clear the mask.  It copies the samples
into the caller's saved register pairs, releases the sample lock, and switches
through ``H2K_interrupt_restore`` so the sampled values are returned to the
caller.  More than six active hardware threads invokes the fatal-kernel path.


H2K_sample_init
---------------

.. c:function:: void H2K_sample_init(void)

Description
~~~~~~~~~~~

Initializes the profiling sample lock.
