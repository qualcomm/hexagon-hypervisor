
:mod:`context` -- thread context definition
===========================================

.. module:: context

This module contains the definition of a thread context.

H2K_thread_context
------------------

Overview
~~~~~~~~

Design of the thread context is important.  For performance, we need to
organize the structure to minimize cache effects.  For total footprint, we need
to minimize the size of the structure.  

First, the thread context has two words available for pointers.  This allows 
the thread context to be used as a singly- or doubly-linked list node.

Next, the thread context has information about the current thread status.
This can include information about the futex being waited on, etc.
We also have information about the thread priority, what hardware thread
the thread is scheduled on, and the state of the thread.

The bulk of the thread context is made up of the state that makes up a thread.
This includes the general registers, user control registers, and some
supervisor registers associated with the thread.  

The thread context also contains information for collecting per-software thread
usage statistics.

The DCZEROA instruction is used to aid in cache performance for saving context.
The DCZEROA instruction clears an aligned 32-byte chunk of memory.  This means
that it is important to carefully place the data such that the bulk of the
context save can be done to memory that has been cleared, but also that the
clear instructions will not clobber values that should be maintained.

Description
~~~~~~~~~~~

.. c:type:: H2K_thread_context

	Thread Context Storage and Control Block

	.. c:member:: H2K_thread_context *next

		Pointer to the next thread context in a list or ring

	.. c:member:: H2K_thread_context *prev

		Pointer to the previous thread context in a ring

	.. c:member:: u8_t prio

		Current priority of the thread

	.. c:member:: u8_t base_prio

		Priority of the thread to be restored after inherited priority is finished

	.. c:member:: u8_t hthread

		If the thread is running, this is the hardware thread it is scheduled on.

	.. c:member:: u8_t status

		Current status of the thread (DEAD, RUNNING, READY, or BLOCKED).

	.. c:member:: u8_t vmstatus

		Current Virtual Machine status of the thread (OK, NEEDS_WORK).

	.. c:member:: u8_t tid

		Software Thread ID value

	.. c:member:: void *gevb

		Guest Event Vector Base, used as a base address for errors, VM exceptions, VM
		interrupts, and other events.

	.. c:member:: u32_t trapmask

		Mask of which traps are valid for this thread to execute.

	.. c:member:: u32_t gbadva

		Guest Bad Virtual Address Register

	.. c:member:: u32_t gelr

		Guest Event Link Register

	.. c:member:: u32_t gosp

		Guest Other Stack Pointer register

	.. c:member:: u32_t gssr

		Guest System Status Register

	.. c:member:: u64_t totalcycles

		Total accumulated cycle count for this thread.

	.. c:member:: u32_t futex_ptr

		Address the thread is currently blocked on.

	.. c:member:: void *continuation

		Code location that will correctly return from the kernel to the thread

	.. c:member:: u64_t ccrssr

		System Status Register and Cache Control Register for the thread

	.. c:member:: u34_t elr

		Event Link Register for the thread

	.. c:member:: u64_t **other_register_storage**

		Other register storage for user general purpose and control registers


H2K_fastint_context
-------------------

A fastint context contains a normal thread context, and additionally has extra
padding on the end for use for the fast interrupt stack.


.. c:type:: H2K_fastint_context

	.. c:member:: H2K_thread_context context

		Normal thread context

	.. c:member:: u64_t **stack_XXX**

		Area to use as a Fast Interrupt stack


H2K_context_save
----------------

.. c:function:: H2K_context_save()

Description
~~~~~~~~~~~

This is a generic context save interface.  Nearly all of the context is saved.
Additionally, the kernel stack and KGP are set up.

Since it is callable, it assumes that r31 and r30 have already been saved, and
r31 holds the return address.  Additionally, since the caller had to save r31
and r30, it assumes that r0 and SGP are swapped.

When we return, r31 holds the value of :c:func:`H2K_context_restore_return()`.
Additionally, r7 holds the current context pointer, r8 holds the SSR, r9 holds
the HTID, r28 holds the KGP, and r29 holds the correct stack pointer. R0 through
r6 are unchanged from the caller.

r30 is not modified


Functionality
~~~~~~~~~~~~~

Saving and Restoring state is done as efficiently as possible, but cache 
optimization may be used to help worst case performance. 


H2K_context_restore_return
--------------------------


.. c:function:: H2K_context_restore_return()

Description
~~~~~~~~~~~

This is a generic context restore interface.  All of the context is restored.

Functionality
~~~~~~~~~~~~~

All of the context is restored from the context.

The function ends with an RTE to go back to leave the monitor.


Testing
-------

For each set of data in the thread context that needs to be saved and restored,
save it with :c:func:`H2K_context_save()`, check that it was placed in the right 
place, and then restore it with :c:func:`H2K_context_restore_return()` and make
sure it was reloaded correctly.



