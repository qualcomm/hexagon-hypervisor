
ASM_REF_CODE(fastint requires special register changes difficult in C)

:mod:`fastint` -- call a fast interrupt handler
===============================================

.. module:: fastint


H2K_fastint
-----------

.. c:function:: H2K_fastint(u32_t intno, H2K_thread_context *me, u32_t hthread)

	:param intno: Interrupt number
	:param me: Context for the current thread
	:param hthread: Current hardware thread number

Description
~~~~~~~~~~~

This function calls a fast interrupt

Functionality
~~~~~~~~~~~~~

The :c:func:`H2K_fastint()` routine sets up the environment for a fast interrupt handler, and
then calls the fast interrupt handler.

When the fast interrupt handler returns, we clear the IAD bit for the fast interrupt.
We then check to see if any interrupts are pending in IPEND.  

If so, we set IMASK to mask off non-fast interrupts, set SSR to enable
interrupts, and execute enough instructions to receive interrupts -- an isync
and a nop.  We then set SSR to disable interrupts, and jump to the context restore.

If IPEND was clear all along, we simply jump to the context restore.

Note that fast interrupt handlers may call traps.  This means that the per-thread
kernel stack and most the thread context cannot be assumed to be unchanged when the 
fast interrupt returns.

However, the fast interrupt handler is not actually scheduled, nor can it
block.  This means that the next and previous pointers are useable for storage.
Similarly, we do not collect timing statistics for fast interrupts, so the
oncpu_start and totalcycles in the per-hardware-thread fast interrupt context
are acceptable for use as temporary storage.

If the fast interrupt handler returns nonzero, the interrupt is re-enabled.
Otherwise, it remains disabled until re-enabled through another mechanism.



Testing
-------


Samples
~~~~~~~

* Input: thread context in SGP, or NULL
* Input: H2K_kg.fastint_contexts
* Input: H2K_kg.inthandlers
* Input: H2K_kg.fastint_gp
* Flow: go to approrpiate fast interrupt handler

Important cases
~~~~~~~~~~~~~~~

* SGP is NULL
* SGP is non-NULL
* Each L1 interrupt
* Interrupt pending for H2K_fastint_return

Harness
~~~~~~~

We will link only with the interrupt object file.  

The harness will have three sections:

* Calling H2K_fastint and checking that the appropriate fastint handler was called


The simple call to H2K_fastint will be possible directly from C.  

The call to H2K_interrupted_fastint_check requires an assembly wrapper to 
set up register values correctly.

The check for the additional fastint being taken will require assembly code to
set up a new vector table, add the interrupt as pending, and to handle the
architecture state correctly.

