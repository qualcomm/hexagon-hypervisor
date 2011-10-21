
ASM_REF_CODE(Trap context save/restore not possible in C)

:mod:`trap` -- handle kernel request events
=============================================

.. module:: trap

H2K_handle_trap0
----------------

.. cfunction:: H2K_handle_trap0()

Description
~~~~~~~~~~~

The trap0 instruction goes to the trap0 vector, which jumps to this routine.

The handle_trap0 function saves callee-save registers according to the ABI,
including special registers.  It also determines the functionality requested
by the trap, and calls the corresponding kernel routine.

Functionality
~~~~~~~~~~~~~

XXX: TBD: for VM, we must not clobber regs...

The H2 Trap API specifies that only the callee-save registers be saved by the
kernel during a trap0.  This allows the kernel to immediately use scratch registers
for saving context and determining the correct behavior for the trap.

In simulation environments, and in environments attached to T32, the program
can use special semihosting routines to request that functionalities like
printing to the console and file I/O be emulated by the simulation or debug
environment.  Semihosting requests use the trap0 immediate 0.  This should be
detected early, and should branch to a label that can be used for detecting the 
semihosting request.  After the branch, we can return immediately.  Note that trap0(#0)
should have no effect (other than clobbered registers) for a node not connected to 
an appropriate semihosting provider.

We then check to ensure that the thread can use the requested trap.  This is
used to reduce capabilities of less-priviledged threads, and is also used to
assert that fast interrupt handlers cannot block.  If a user-mode thread cannot
execute the trap, we call :cfunc:`H2K_fatal_thread()`.  If a fast interrupt cannot
execute the trap, we call :cfunc:`H2K_fatal_kernel()`.

We then call a stub that saves the return value into the continuation field, 
and jump to the traptab entry for the requested trap.  

The trap request can return, or can call the trap continuation.


H2K_traptab
-----------

.. cfunction:: H2K_traptab()

Description
~~~~~~~~~~~

The traptab is a table of pairs of instructions.  One instruction of the pair
is the jump instruction to the appropriate place for the trap of the appropriate
index, and the other instruction in the pair transfers the pointer to the
current thread context to the appropriate argument register, as this is an
argument for most functions.

To go to the appropriate handler for a trap, jump to traptab + 8*trap_number.

Functionality
~~~~~~~~~~~~~

Each pair of instructions corresponds to the appropriate trap id.
Several traps handlers may point to :cfunc:`H2K_thread_id()`, which 
is used wherever empty traptab spaces exist.  The table is as follows:

	0. :cfunc:`H2K_trap_angel()`: dummy handler for ANGEL semihosting
	1. :cfunc:`H2K_thread_id()`: Get the thread ID
	2. :cfunc:`H2K_futex_wait()`: Wait on a futex
	3. :cfunc:`H2K_futex_resume()`: Wake threads waiting on a futex
	4. :cfunc:`H2K_thread_create()`: Create a new thread
	5. :cfunc:`H2K_thread_stop()`: Stop and recycle the current thread
	6. :cfunc:`H2K_cputime_get()`: Get CPU time
	7. :cfunc:`H2K_thread_id()`: Get the thread ID
	8. :cfunc:`H2K_register_fastint()`: Register a fast interrupt handler
	9. :cfunc:`H2K_prio_set()`: Change priority
	10. :cfunc:`H2K_prio_get()`: Get priority
	11. :cfunc:`H2K_popup_wait()`: Block, waiting on an interrupt
	12. :cfunc:`H2K_sched_yield()`: Yield to another thread at the same priority
	13. :cfunc:`H2K_thread_id()`: Get the thread ID
	14. :cfunc:`H2K_thread_id()`: Get the thread ID
	15. :cfunc:`H2K_thread_id()`: Get the thread ID
	16. :cfunc:`H2K_pcycles_get()`: Get processor cycle count
	17. :cfunc:`H2K_thread_id()`: Get the thread ID
	18. :cfunc:`H2K_tid_set()`: Set software thread ID
	19. :cfunc:`H2K_tid_get()`: Get software thread ID
	20. :cfunc:`H2K_futex_lock_pi()`: Priority Inheritance Futexy-Mutex Lock
	21. :cfunc:`H2K_futex_unlock_pi()`: Priority Inheritance Futexy-Mutex Unlock
	22. :cfunc:`H2K_thread_id()`: Get the thread ID
	23. :cfunc:`H2K_thread_id()`: Get the thread ID
	24. :cfunc:`H2K_thread_id()`: Get the thread ID
	25. :cfunc:`H2K_thread_id()`: Get the thread ID
	26. :cfunc:`H2K_thread_id()`: Get the thread ID
	27. :cfunc:`H2K_waitcycles_get()`: Get waitcycles for a hardware thread
	28. :cfunc:`H2K_vmboot()`: Start VM
	29. :cfunc:`H2K_trap_pmuconfig()`: Adjust PMU Configuration
	30. :cfunc:`H2K_trap_config()`: Adjust Kernel Configuration
	31. :cfunc:`H2K_trap_hwconfig()`: Adjust Hardware Configuration


Testing
-------


Samples
~~~~~~~

* Input: thread context in SGP

Important cases
~~~~~~~~~~~~~~~

* Trap enabled
* Trap disabled

* Every cause code

Harness
~~~~~~~

We will link only with the trap object file.

The harness will have a helper function:

.. cfunction:: void TH_do_trap(H2K_thread_context *src, H2K_thread_context *dest, u32_t trapnum)

This function will load the appropriate registers from the src thread context, set
SGP to the storage pointed to by `dest`, and call :cfunc:`H2K_handle_trap0()` with the
correct SSR CAUSE code for the trap corresponding to num having happened.

The test harness will define each possible trap handler as a check to make sure any `me` value 
is set correctly, and then will call:

.. cfunction::  void TH_check_trap(H2K_thread_context *src, H2K_thread_context *dest)

This function will check to make sure that the appropriate registers from `src` and `dest`
are equal, to check that the context was saved correctly.  It will also check to make 
sure that the continuation is set correctly, and that the stack is set up correctly.

