
:mod:`thread_id` -- obtain unique ID
====================================

.. module:: thread_id

H2K_thread_id
-------------

.. cfunction:: u32_t H2K_thread_id(H2K_thread_context *me)

	:param me: Pointer to the current context
	:returns: A unique ID for the running thread

Description
~~~~~~~~~~~

The :cfunc:`H2K_thread_id()` function returns a unique id for the requesting thread.

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


Testing
-------


Samples
~~~~~~~

* Input: `me` 
* Output: unique ID (Currently, should be equal to `me`)

Important cases
~~~~~~~~~~~~~~~

* Various values of `me`

Harness
~~~~~~~

We link directly with the thread object file.

The test harness calls :cfunc:`H2K_thread_id()` with `me`, and expects the return value
to be equal to the thread context pointer.

