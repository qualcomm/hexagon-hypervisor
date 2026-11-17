:mod:`thread_create` -- Create a virtual CPU thread
======================================================

.. module:: thread_create

H2K_thread_create_no_squash
---------------------------

.. c:function:: s32_t H2K_thread_create_no_squash(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me)

	:param pc: Initial program counter
	:param sp: Initial stack pointer
	:param arg1: Initial value for ``r0``
	:param prio: Requested priority
	:param vmblock: VM block for the new thread
	:param me: Current thread context
	:returns: New thread ID, or -1 on failure

Description
~~~~~~~~~~~

Allocates and initializes a thread context for the supplied VM.

Functionality
~~~~~~~~~~~~~

Rejects an out-of-range priority, a priority better than the VM's
``bestprio``, an unaligned stack, or an unaligned PC.  Under the BKL, it takes
a context from ``vmblock->free_threads`` and initializes its registers,
priority, trap mask, TLB index mask, continuation, guest entry state, and
address-space ID.  The first CPU gets an identity offset mapping; later CPUs
inherit the current thread's translation information.

The new context is attached to the VM, the VM CPU count is incremented, and
the context is appended to the ready list.  Failure to allocate an ASID
returns the context to the free list and returns -1.


H2K_thread_create
-----------------

.. c:function:: s32_t H2K_thread_create(u32_t pc, u32_t sp, u32_t arg1, u32_t prio, H2K_vmblock_t *vmblock, H2K_thread_context *me)

	:param pc: Initial program counter
	:param sp: Initial stack pointer
	:param arg1: Initial value for ``r0``
	:param prio: Requested priority
	:param vmblock: Ignored; the new thread uses ``me->vmblock``
	:param me: Current thread context
	:returns: New thread ID, or -1 on failure

Description
~~~~~~~~~~~

Creates a thread in the current thread's VM by forwarding to
:c:func:`H2K_thread_create_no_squash()` with ``me->vmblock``.

Testing
-------

Important cases include invalid priority, priority above the VM limit,
unaligned PC or stack, an empty free-thread list, ASID allocation failure, and
successful initialization and insertion into the ready list.
