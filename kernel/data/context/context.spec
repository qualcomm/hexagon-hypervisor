
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

.. ctype:: H2K_thread_context

	Thread Context Storage and Control Block

	.. cmember:: H2K_thread_context *next

		Pointer to the next thread context in a list or ring

	.. cmember:: H2K_thread_context *prev

		Pointer to the previous thread context in a ring

	.. cmember:: u8_t prio

		Current priority of the thread

	.. cmember:: u8_t baseprio

		Base priority of the thread, not including adjustments from priority 
		inversion avoidance.

	.. cmember:: u8_t schedprio

		Priority the thread was scheduled at; this determines where in the 
		H2K_ready structure the thread is located.

	.. cmember:: u8_t hthread

		If the thread is running, this is the hardware thread it is scheduled on.

	.. cmember:: u8_t status

		Current status of the thread (DEAD, RUNNING, READY, or BLOCKED).

	.. cmember:: u8_t vmstatus

		Current Virtual Machine status of the thread (OK, NEEDS_WORK).

	.. cmember:: u8_t tid

		Software Thread ID value

	.. cmember:: void *gevb

		Guest Event Vector Base, used as a base address for errors, VM exceptions, VM
		interrupts, and other events.

	.. cmember:: u32_t trapmask

		Mask of which traps are valid for this thread to execute.

	.. cmember:: u32_t gbadva

		Guest Bad Virtual Address Register

	.. cmember:: u32_t gelr

		Guest Event Link Register

	.. cmember:: u32_t gosp

		Guest Other Stack Pointer register

	.. cmember:: u32_t gssr

		Guest System Status Register

	.. cmember:: u64_t oncpu_start

		Cycle count when the thread started execution.  Valid only when the thread
		`status` is RUNNING.

	.. cmember:: u64_t totalcycles

		Total accumulated cycle count for this thread.

	.. cmember:: u32_t futex_ptr

		Address the thread is currently blocked on.

	.. cmember:: void *continuation

		Code location that will correctly return from the kernel to the thread

	.. cmember:: u64_t ssrelr

		System Status Register and Event Link Register for the thread

	.. cmember:: u64_t **other_register_storage**

		Other register storage for user general purpose and control registers


H2K_fastint_context
-------------------

A fastint context contains a normal thread context, and additionally has extra
padding on the end for use for the fast interrupt stack.


.. ctype:: H2K_fastint_context

	.. cmember:: H2K_thread_context context

		Normal thread context

	.. cmember:: u64_t **stack_XXX**

		Area to use as a Fast Interrupt stack

