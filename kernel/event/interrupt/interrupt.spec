
ASM_REF_CODE(Interrupt context save/restore not possible in C)

:mod:`interrupt` -- Handle interrupt events
===========================================

.. module:: interrupt

H2K_handle_int
--------------

.. cfunction:: H2K_handle_int()

Description
~~~~~~~~~~~

This handles the common set of interrupts.  The interrupt handler carefully
saves any required context, and jumps to the appropriate handler for the 
next step.

Functionality
~~~~~~~~~~~~~

Most interrupts redirect to handle_int.

Handle_int carefully saves off any required state.

If the idle thread was interrupted, we detect the case as early as possible and
skip the remainder of the context save.  If we have interrupted the idle
thread, we do not care about saving the values of the registers.

If the fast interrupt handler "interrupt check" was interrupted, we vector off
to the approrpate code to handle it.  See fast interrupts for more information.

Interrupts can be handled by calling the reschedule function, or calling the
function to handle fast interrupts.  The fast interrupt handler may be called
before saving the registers saved by the ABI to improve performance.

What function to call is determined by the table H2K_inthandlers.  The format
of each inthandler should be::

	H2K_handler(u32_t intno, H2K_thread_context *me, u32_t hwtnum)

Real interrupted threads (as opposed to, for example, the idle thread) that may 
reschedule should have int_context_restore as the end of their continuation
function.

We leave to execute the interrupt handler that calls a stub routine.  This stub
routine saves the return address in the continuation field before jumping to
the appropriate location.

H2K_int_context_restore
-----------------------

.. cfunction:: H2K_int_context_restore()

Description
~~~~~~~~~~~

This routine returns to an interrupted thread.  It restores the registers from
the thread context and returns to the thread in the appropriate place.

Functionality
~~~~~~~~~~~~~

The int_context_restore routine restores registers from the thread context and returns.


H2K_interrupted_waitmode
------------------------

.. cfunction:: H2K_interrupted_waitmode()

Description
~~~~~~~~~~~

This routine is called when we detect that we were interrupted while in wait.  
This is a common case, and we can avoid context save by branching to this 
routine early.

Functionality
~~~~~~~~~~~~~

We set up the stack pointer and call the correct function.


Testing
-------


Samples
~~~~~~~

* Input: thread context in SGP, or NULL
* Input: H2K_inthandlers
* Output: All (?) registers should be saved in thread context if SGP is non-NULL
* Flow: go to event handler for event number

Important cases
~~~~~~~~~~~~~~~

* SGP is NULL
* SGP is non-NULL
* SGP is fast interrupt handler
* Each L1 interrupt

Harness
~~~~~~~

We will link only with the interrupt object file.  

The harness will have a helper function:

.. cfunction:: void TH_do_interrupt(H2K_thread_context *src, H2K_thread_context *dest, u32_t num)

This function will load the appropriate registers from the src thread context, set
SGP to the storage pointed to by `dest`, and call :cfunc:`H2K_handle_int()` with the
correct SSR CAUSE code for the interrupt corresponding to num having happened.

For non-NULL SGP value tests, the approriate entry in H2K_inthandlers will point to a second helper function:

.. cfunction::  void TH_check_interrupt(H2K_thread_context *src, H2K_thread_context *dest)

This function will check to make sure that the appropriate registers from `src` and `dest`
are equal, to check that the context was saved correctly.  It will also check to make 
sure that the continuation is set correctly, the stack is set up correctly, and the argument
registers are correct.

For NULL SGP value tests, the correct entry H2K_inthandlers will point to a simpler check for 
the stack and arguments.

All incorrect entries in H2K_inthandlers should be set to code that calls FAIL.

