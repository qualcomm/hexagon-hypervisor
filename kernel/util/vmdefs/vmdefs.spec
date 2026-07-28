:mod:`vmdefs` -- Virtual Machine Compile-Time Definitions
=========================================================

.. module:: vmdefs

This unit contains compile-time definitions for C and Assembly that
are needed for the Guest Architecture: VM status bits, GSSR bits, the
supported VM ABI version, GEVB offsets, and VMBLOCK flags.  The
substantive semantics for the asynchronous-work signaling protocol are
documented here too, since the bit definitions are the natural home
for the contract.


VMSTATUS bits
-------------

The ``vmstatus`` byte in :c:type:`H2K_thread_context` carries
asynchronous signaling state for the thread.  Bits are defined in
``vmdefs.h``; their semantics are documented below.

.. c:macro:: H2K_VMSTATUS_VMWORK

    The VMWORK bit -- the canonical "reconsider before blocking" kick.
    See :ref:`vmwork-contract` below.

.. c:macro:: H2K_VMSTATUS_KILL

    The thread is marked for termination.  Whenever a thread sets KILL
    on another thread it must also set VMWORK in the same operation,
    so that the gate at every block point routes the victim into
    :c:func:`H2K_vm_do_work_withlock()` where the KILL is honored and
    the thread is stopped (never returns).

    KILL is asynchronous: the setter does not wait for the victim to
    notice.  The victim notices either at its next trap return (the
    deferred-work hook in :c:func:`H2K_switch`), or at the next block
    primitive it enters (the VMWORK gate).

.. c:macro:: H2K_VMSTATUS_IE

    Guest interrupt-enable mirror.  When set, the guest has interrupts
    enabled and pending interrupts may be delivered.  When clear, the
    guest has masked interrupts; pending interrupts remain in the
    interrupt controller and are re-discovered when the guest
    re-enables interrupts via :c:func:`H2K_enable_guest_interrupts`.

    IE is **not** part of the VMWORK gate -- a thread with VMWORK set
    must not block regardless of IE state.

.. c:macro:: H2K_VMSTATUS_SAVEXT

    Extended registers (HVX/HMX/HLX) are live for this thread and must
    be saved/restored on context switch.  Unrelated to the
    asynchronous-work protocol.


.. _vmwork-contract:

The VMWORK contract
-------------------

VMWORK is the asynchronous-work signaling bit.  Any kernel code that
performs work on behalf of a thread T from another hardware thread --
killing T, delivering an interrupt to T, etc. -- sets VMWORK on T so
that T will reconsider its state before entering (or while entering) a
blocked state.

The contract has two halves.

**Strong guarantee #1 -- the no-block rule:**

  A thread MUST NOT enter a blocked state while VMWORK is set in its
  ``vmstatus``.

  Every blocking primitive (``H2K_futex_wait``, ``H2K_futex_lock_pi``,
  ``H2K_popup_wait``, ``H2K_intpool_wait``, and any future addition)
  must check VMWORK after it has acquired the BKL but before it
  commits the thread to ``STATUS_BLOCKED`` / ``STATUS_INTBLOCKED``.
  If VMWORK is set the primitive has two acceptable responses:

  a. **Bail spuriously.**  Release the BKL, return ``-1`` to the
     caller without setting the blocked status.  The caller is
     required to treat ``-1`` as "operation did not happen, re-check
     wait condition and retry" -- the same way it would treat any
     other spurious failure (TLB miss, etc.).  This is the response
     used today by ``H2K_futex_wait`` and ``H2K_futex_lock_pi``.

  b. **Handle the work inline.**  Call into
     :c:func:`H2K_vm_do_work_withlock()` (or do equivalent work),
     which will honor KILL if set (never returns) and clear VMWORK,
     then proceed with the block.

  The bail response is preferred when there is no natural place to
  resume after handling -- spurious-retry is already a required
  caller-side capability, and bailing keeps the block primitives
  simple.

**Strong guarantee #2 -- the clear-is-a-promise rule:**

  Clearing the VMWORK bit is a promise that all pending asynchronous
  VM work has been considered.

  "Considered" does not mean "dispatched" -- in some cases the
  conclusion is that no action is needed *right now*.  For example, a
  masked interrupt (guest IE clear) does not get delivered when VMWORK
  fires; the interrupt remains in the interrupt controller and is
  re-discovered via :c:func:`H2K_vm_check_interrupts` when the guest
  re-enables IE.  Clearing VMWORK in this case is safe because the
  substantive state lives elsewhere.

  The canonical place where VMWORK gets cleared is
  :c:func:`H2K_vm_do_work_withlock()`.  It clears VMWORK
  unconditionally (after the KILL check, which never returns), because
  every reason VMWORK could have been set has either been dispatched
  by that function or is preserved in a substantive location (the
  interrupt controller, the KILL bit, etc.).

**What VMWORK is not:**

  VMWORK is not itself a unit of work -- it is a hint.  The
  substantive work always lives elsewhere: the KILL bit, the interrupt
  controller, a pending IPI cause, etc.  Losing the VMWORK bit cannot
  lose work; failing to honor the bit (blocking with it set) loses the
  thread's chance to notice the work and is the actual hazard.


Return value to the guest on spurious failure
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

When a block primitive bails spuriously its return value flows back to
the guest in r0 via the trap-return path
(:c:func:`H2K_handle_trap0` -> ``H2K_trap_done`` writes r0/r1 from the
returning primitive into the context, then ``H2K_context_restore``
delivers it).  Guest-side wrappers must therefore handle a ``-1``
return from a blocking syscall by re-checking the wait condition and
retrying -- they cannot assume that any return from the syscall means
the thread successfully blocked and was woken.

Note that the post-block wake path is *different*: a thread that
actually blocked is later woken via :c:func:`H2K_switch` and resumes
through ``H2K_context_restore`` with r0 already set by whoever woke
it.  The trap_done r0/r1 writeback only governs the
synchronous-return (spurious-failure) case discussed here.


GSSR bits
---------

.. c:macro:: H2K_GSSR_UM

    User Mode bit in the Guest SSR.

.. c:macro:: H2K_GSSR_IE

    Interrupt Enable bit in the Guest SSR.

.. c:macro:: H2K_GSSR_SS

    Supervisor Stack bit in the Guest SSR.


Other definitions
-----------------

GEVB offsets (``RESET_GEVB_OFFSET`` through ``INTERRUPT_GEVB_OFFSET``)
locate the per-cause entry points within the Guest Event Vector Base.

``H2K_VM_VERSION`` is the supported VM ABI version exposed to guests.

``H2K_VMBLOCK_FLAGS_USE_EXT_BIT`` and ``H2K_VMBLOCK_FLAGS_DO_EXT_BIT``
control extended-register save behavior on blocking operations.
