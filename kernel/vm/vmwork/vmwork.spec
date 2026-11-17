:mod:`vmwork` -- Virtual machine deferred work
================================================

.. module:: vmwork


H2K_vm_do_work_withlock
-----------------------

.. c:function:: s32_t H2K_vm_do_work_withlock(H2K_thread_context *me) IN_SECTION(".text.vm.work")

	:param me: Current VCPU context
	:returns: Result from :c:func:`H2K_vm_check_interrupts()`, or ``-1`` when no VM work is pending

Handles deferred work for a VCPU while the caller holds the BKL.  If the
``H2K_VMSTATUS_KILL`` bit is set, calls
:c:func:`H2K_thread_stop_withlock()` with the VM's status and the VCPU context;
the normal kill path does not return.  Otherwise, if the
``H2K_VMSTATUS_VMWORK`` bit is clear, returns ``-1`` without changing the
context.

When VMWORK is set, atomically clears the VMWORK bit and calls
:c:func:`H2K_vm_check_interrupts()`.  That check either consumes and raises a
pending interrupt when guest interrupts are enabled, or only observes a
pending interrupt when they are disabled.  The result of that check is returned
to the caller.
