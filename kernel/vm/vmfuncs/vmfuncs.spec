:mod:`vmfuncs` -- VM trap functions
====================================

.. module:: vmfuncs

These functions operate on the current virtual CPU context.  Results are
returned by updating registers in ``me`` rather than through the C return value.


H2K_vmtrap_version
------------------

.. c:function:: void H2K_vmtrap_version(H2K_thread_context *me)

	:param me: Current thread context

Places ``H2K_VM_VERSION`` in ``me->r00``.


H2K_vmtrap_return
-----------------

.. c:function:: void H2K_vmtrap_return(H2K_thread_context *me)

	:param me: Current thread context

Restores the guest register state and returns from a VM event using the saved
``GELR`` and ``GSSR`` values.  When the saved user-mode bit is set, swaps the
guest and kernel stack pointers and clears the guest-mode bit in the live SSR.
It also restores the saved single-step state.  If guest interrupts were enabled
in the saved state, re-enables them through
:c:func:`H2K_enable_guest_interrupts()`.


H2K_vmtrap_setvec
-----------------

.. c:function:: void H2K_vmtrap_setvec(H2K_thread_context *me)

	:param me: Current thread context

Sets ``me->gevb`` from ``me->r00`` and sets ``me->r00`` to zero on success.


H2K_vmtrap_setie
----------------

.. c:function:: void H2K_vmtrap_setie(H2K_thread_context *me)

	:param me: Current thread context

Sets the virtual interrupt-enable state according to ``me->r00`` and returns
the previous state in ``me->r00``.  If the requested value is outside the
supported ``H2K_IE`` range, returns ``-1`` in ``me->r00`` without changing the
state.  Otherwise, the ``H2K_IE_ENABLE`` bit selects
:c:func:`H2K_enable_guest_interrupts()` or
:c:func:`H2K_disable_guest_interrupts()`.


H2K_vmtrap_getie
----------------

.. c:function:: void H2K_vmtrap_getie(H2K_thread_context *me)

	:param me: Current thread context

Places the current virtual interrupt-enable state in ``me->r00``.


H2K_vmtrap_get_pcycles
----------------------

.. c:function:: void H2K_vmtrap_get_pcycles(H2K_thread_context *me)

	:param me: Current thread context

Places the current virtual CPU time in ``me->r0100`` and the current hardware
pcycle value in ``me->r0302``.


H2K_vmtrap_set_pcycles
----------------------

.. c:function:: void H2K_vmtrap_set_pcycles(H2K_thread_context *me)

	:param me: Current thread context

Sets ``me->totalcycles`` from ``me->r0100`` and resets the current hardware
thread's ``oncpu_start`` value to the current pcycle register.  This operation
is retained for compatibility; a guest can otherwise track elapsed time using
deltas from :c:func:`H2K_vmtrap_get_pcycles()`.


H2K_vmtrap_wait
---------------

.. c:function:: void H2K_vmtrap_wait(H2K_thread_context *me)

	:param me: Current thread context

Runs pending VM work while holding the BKL.  If work produces an interrupt,
returns its number in ``me->r00`` and releases the BKL.  If no work is pending,
marks the virtual CPU as ``VMWAIT``, records it as waiting when its CPU index is
within the tracked waiting-CPU mask, removes it from the run list, and
reschedules it.  The blocked path does not return immediately to the caller.


H2K_vmtrap_yield
----------------

.. c:function:: void H2K_vmtrap_yield(H2K_thread_context *me)

	:param me: Current thread context

Calls :c:func:`H2K_sched_yield()` and places zero in ``me->r00``.


H2K_vmtrap_start
----------------

.. c:function:: void H2K_vmtrap_start(H2K_thread_context *me)

	:param me: Current thread context

Creates a VCPU by calling :c:func:`H2K_thread_create_no_squash()`.  The new
VCPU's start address is ``me->r00``, stack pointer is ``me->r01``, argument is
zero, and priority is the caller's base priority.  The new VCPU belongs to the
caller's VM block.  The creation result is returned in ``me->r00``.


H2K_vmtrap_stop
---------------

.. c:function:: void H2K_vmtrap_stop(H2K_thread_context *me)

	:param me: Current thread context

Calls :c:func:`H2K_thread_stop()` for the thread identifier in ``me->r00``.


H2K_vmtrap_vmpid
----------------

.. c:function:: void H2K_vmtrap_vmpid(H2K_thread_context *me)

	:param me: Current thread context

Places the raw virtual-CPU identifier returned by
:c:func:`H2K_id_from_context()` in ``me->r00``.


H2K_vmtrap_setregs
------------------

.. c:function:: void H2K_vmtrap_setregs(H2K_thread_context *me)

	:param me: Current thread context

Copies the guest register pair values from ``me->r0100`` and ``me->r0302``
into the saved guest state: ``GELR/GSSR`` and ``GOSP/GBADVA``, respectively.
Then restores the guest register state.


H2K_vmtrap_getregs
------------------

.. c:function:: void H2K_vmtrap_getregs(H2K_thread_context *me)

	:param me: Current thread context

Saves the current guest register state and returns ``GELR/GSSR`` in
``me->r0100`` and ``GOSP/GBADVA`` in ``me->r0302``.


H2K_vmtrap_pmuctrl
------------------

.. c:function:: void H2K_vmtrap_pmuctrl(H2K_thread_context *me)

	:param me: Current thread context

Normally forwards ``r00`` through ``r03`` to
:c:func:`H2K_trap_pmuctrl()` and places its result in ``r00``.  For
``PMUCTRL_THREADSET`` with a raw VCPU identifier of zero, applies the operation
to every VCPU in the current VM block, ignores errors from dead VCPUs, and
returns the number of successful operations in ``r00``.


H2K_vmtrap_info
---------------

.. c:function:: void H2K_vmtrap_info(H2K_thread_context *me)

	:param me: Current thread context

Forwards ``r00``, ``r01``, and ``r02`` to
:c:func:`H2K_trap_info()` as the information selector, selector argument, and
configuration-unit entry, respectively.  Places the result in ``me->r00``.


Testing
-------

Test the register results and state changes for each operation.  In particular,
exercise VM return state restoration, interrupt enable/disable and invalid
input, wait with and without pending work, VCPU creation and stopping, guest
register transfer, and the PMU and information forwarding paths.
