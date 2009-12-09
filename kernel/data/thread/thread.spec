
:mod:`thread` -- data structures for threads
=============================================

.. module:: thread

This module contains the data structures for threads.

H2K_free_threads
----------------

H2K_free_threads is a linked list of all threads ready for use.


H2K_thread_context_clear
------------------------

.. cfunction:: void H2K_thread_context_clear(H2K_thread_context *thread)

Description
~~~~~~~~~~~

Zeros a thread context.

Input
~~~~~

Argument 0: pointer to a thread context

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

Sets every bit in the thread context to zero.

