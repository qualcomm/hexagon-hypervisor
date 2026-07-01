:mod:`vmwork` -- Virtual Machine Deferred Work
==============================================

.. module:: vmwork



H2K_vm_do_work
--------------

.. c:function:: s32_t H2K_vm_do_work(H2K_thread_context *me)

	:param me: Pointer to the current thread context


Description
~~~~~~~~~~~

This function handles work that is deferred, typically because
the work was done on another hardware thread or while the VCPU
was unscheduled.

The BKL must NOT be held on entry.  ``H2K_vm_do_work`` acquires the BKL,
calls :c:func:`H2K_vm_do_work_withlock()`, and releases the BKL on return.
Callers that already hold the BKL must call
:c:func:`H2K_vm_do_work_withlock()` directly.

Functionality
~~~~~~~~~~~~~

Acquires the BKL, invokes :c:func:`H2K_vm_do_work_withlock()`, and releases
the BKL.  The actual work is documented under that function.


H2K_vm_do_work_withlock
-----------------------

.. c:function:: s32_t H2K_vm_do_work_withlock(H2K_thread_context *me)

	:param me: Pointer to the current thread context


Description
~~~~~~~~~~~

The body of :c:func:`H2K_vm_do_work()` for callers that already hold the
BKL.  Notable callers include the deferred-work hook in ``H2K_switch``
(between :c:func:`H2K_dosched()` and ``H2K_check_sanity_unlock``) and
:c:func:`H2K_vmtrap_vmwait()`.

INPUT_ASSERT(kernel_locked)

Functionality
~~~~~~~~~~~~~

This function is the canonical handler for the ``H2K_VMSTATUS_VMWORK``
bit.  Per the VMWORK contract (see :doc:`../../util/vmdefs/vmdefs`),
clearing VMWORK is a promise that all pending asynchronous VM work has
been considered.  The substantive work for each cause lives elsewhere
(the interrupt controller, the ``H2K_VMSTATUS_KILL`` bit, etc.); VMWORK
itself is only the kick that says "reconsider before blocking."

There are only a few reasons why a VCPU would have deferred work:

* The VCPU should be killed.  If ``H2K_VMSTATUS_KILL`` is set, we call
  :c:func:`H2K_thread_stop_withlock()` and never return.  VMWORK is
  always set whenever KILL is set, so the gate at every block point
  routes here.

* The VCPU may have an interrupt to take.  ``H2K_VMSTATUS_VMWORK`` is
  cleared unconditionally and we route to
  :c:func:`H2K_vm_check_interrupts()`.  If guest interrupts are
  disabled, no interrupt is delivered now; the pending interrupt
  remains in the interrupt controller and is re-discovered when the
  guest re-enables ``H2K_VMSTATUS_IE``, so clearing the bit here
  cannot lose work.

Interrupts are checked by the :c:func:`H2K_vm_check_interrupts()`
function.

