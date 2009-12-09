
:mod:`thread_id` -- obtain unique ID
======================================

.. module:: thread_id

H2K_thread_id
-------------

.. cfunction:: u32_t H2K_thread_id(thread_context *me)

Description
~~~~~~~~~~~

The H2K_thread_id function returns a unique id for the requesting thread.

Input
~~~~~

Argument 0: Pointer to the current context

Output
~~~~~~

A unique ID for the running thread

Functionality
~~~~~~~~~~~~~

Each thread should have a unique ID.

We currently use the pointer to the thread context as the Thread ID.

This helps us change priority of other threads.

Alternatively, if thread contexts must be contiguous in virtual memory, we
could have an index.

Alternatively, we could have a globally-incrementing counter.  However, it
makes it difficult to map id->pointer.  On the other hand, this is rarely
used (currently, mainly for changing priority of another thread).


