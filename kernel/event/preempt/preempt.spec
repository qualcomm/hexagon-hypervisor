ASM_REF_CODE(preempt requires special register changes difficult in C)

:mod:`preempt` -- Generic Preemption Point Support
==================================================

.. module:: preempt


H2K_preempt
-----------

.. c:function:: void H2K_preempt()


Description
~~~~~~~~~~~

This function allows the kernel to take an interrupt.  If preempted, the thread pointed
to by SGP will be continued by calling the continuation.

Functionality
~~~~~~~~~~~~~

The :c:func:`H2K_preempt()` routine attempts to take an interrupt from the
Kernel, if it may be beneficial to do so, as if the currently scheduled task
was running.  This avoids the bulk of the context restore and save, and lowers
worst-case interrupt latency.

In the event that no interrupt was taken, the :c:func:`H2K_preempt()` routine
may return.  Otherwise, it will continue by calling the continuation function
in the current task.

It is important to note that the current task may have been interrupted, so no
context should be lost.  It is also important to note that the interrupt that
may be taken could cause a context switch, so no register context is guaranteed
when the continuation is called.

However, traps that may take a long time typically have available context that
is not required, and can use the free context to hold values across preemption
points.

While checking for an interrupt, we use the value of 0x00000001 in SGP to indicate
that the interrupt was taken from the preemption point.

In the case that the interrupt is taken, the interrupt handler
:c:func:`H2K_handle_int()` must be careful to observe that the SGP indicates a
preemption, and then that the thread context is located in r0.

Testing
-------


Samples
~~~~~~~

* Input: thread context in SGP, or NULL
* Input: interrupts enabled or not
* Input: pending interrupt or not
* Flow: return
* Flow: take interrupt

Important cases
~~~~~~~~~~~~~~~

* SGP is NULL
* SGP is non-NULL
* Interrupt pending 
* Interrupt not pending 
* Interrupts enabled
* Interrupts not enabled (from IMASK due to priority of scheduled task)

Harness
~~~~~~~

We will link only with the preempt object file.  

We will set up a new vector table and link the various interrupts to hook back to
the appropriate test harness code.  We will then enable interrupts.

We will cross SGP values, interrupts enabled or not enabled (via IMASK), and 
various interrupts.  

In each case, we check to see whether or not the interrupt was taken as 
expected.

