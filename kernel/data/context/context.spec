
:mod:`context` -- thread context definition
===========================================

.. module:: context

This module contains the definition of a thread context.

H2K_thread_context
------------------

Description
~~~~~~~~~~~

A thread context contains several parts.

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



A fastint context additionally has extra padding on the end for use for the
fast interrupt stack.


