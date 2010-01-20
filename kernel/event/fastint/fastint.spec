
ASM_REF_CODE(fastint requires special register changes difficult in C)

:mod:`fastint` -- call a fast interrupt handler
===============================================

.. module:: fastint


H2K_fastint
-----------

.. cfunction:: H2K_fastint(u32_t intno, H2K_thread_context *me, u32_t hthread)

	:param intno: Interrupt number
	:param me: Context for the current thread
	:param hthread: Current hardware thread number

Description
~~~~~~~~~~~

This function calls a fast interrupt

Functionality
~~~~~~~~~~~~~

The :cfunc:`H2K_fastint()` routine sets up the environment for a fast interrupt handler, and
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

TBD: should fast interrupts be able to jump to resched also?  Handy for the
commonish case that a fastint causes a reschedule event...


H2K_interrupted_fastint_check
-----------------------------

.. cfunction:: H2K_interrupted_fastint_check()

Description
~~~~~~~~~~~

This routine gets called from handle_int when we detect that the check at the end of 
the fastint handler was interrupted.  Call the correct fastint handler.

Functionality
~~~~~~~~~~~~~

This function depends on knowing the state at the time of the interrupt. 

SGP and R0 have been swapped by the context save routine.

IMASK needs to be restored to the value in R13

Precomputed SSR value for the fast interrupt is in R14

Registers r28-r31 should be restored from the context.  The restored r31
contains the correct return address for the fast interrupt, which means that a
JUMPR instruction will return to the correct place.

Register r4 contains the address of the fast interrupt function pointers

The function uses the known values to compute the correct fast interrupt to jump to,
and makes the jump.  The function will return to the correct location in fastint_call.





Testing
-------


Samples
~~~~~~~

* Input: thread context in SGP, or NULL
* Input: H2K_fastint_contexts
* Input: H2K_fastint_funcptrs
* Input: H2K_fastint_gp
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

* Calling H2K_interrupted_fastint_check with the approrpiate register setup, and checking
  that the appropriate fastint handler was called.

* Calling H2K_fastint with an additional fastint is pending, and checking that the interrupt
  was taken.


The simple call to H2K_fastint will be possible directly from C.  

The call to H2K_interrupted_fastint_check requires an assembly wrapper to 
set up register values correctly.

The check for the additional fastint being taken will require assembly code to
set up a new vector table, add the interrupt as pending, and to handle the
architecture state correctly.

