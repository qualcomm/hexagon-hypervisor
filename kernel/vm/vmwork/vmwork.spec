:mod:`vmwork` -- Virtual Machine Deferred Work
==============================================

.. module:: vmwork



H2K_vm_do_work
--------------

.. cfunction:: s32_t H2K_vm_do_work(H2K_thread_context *me)

	:param me: Pointer to the current thread context


Description
~~~~~~~~~~~

This function handles work that is deferred, typically because
the work was done on another hardware thread or while the VCPU
was unscheduled.

Functionality
~~~~~~~~~~~~~

There are only a few reasons why a VCPU would have deferred work:

* The VCPU should be killed

* The VCPU may have an interrupt to take

We check for these scenarios and handle them appropriately.

Interrupts are checked by the :cfunc:`H2K_vm_check_interrupts()` 
function.

