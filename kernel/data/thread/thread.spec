
:mod:`thread` -- data structures for threads
============================================

.. module:: thread

This module contains the data structures for threads.

H2K_free_threads
----------------

H2K_free_threads is a linked list of all threads ready for use.


H2K_thread_context_clear
------------------------

.. cfunction:: void H2K_thread_context_clear(H2K_thread_context *thread)

	:param thread: pointer to a thread context

Description
~~~~~~~~~~~

Zeros a thread context.

Functionality
~~~~~~~~~~~~~

Sets every bit in the thread context to zero.

H2K_thread_init
---------------

.. cfunction:: void H2K_thread_init(void)

Description
~~~~~~~~~~~

Initializes thread-related data structures.

Functionality
~~~~~~~~~~~~~

Clears out the H2K_boot_context by calling :cfunc:`H2K_thread_context_clear()`.
It also sets H2K_free_threads to NULL.

