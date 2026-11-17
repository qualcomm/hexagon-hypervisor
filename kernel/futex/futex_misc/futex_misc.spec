
:mod:`futex_misc` -- Futex Misc Routines
========================================

.. module:: futex_misc

H2K_futex_cancel
----------------

.. c:function:: void H2K_futex_cancel(H2K_thread_context *dest)

	:param dest: the thread to cancel

Description
~~~~~~~~~~~

:c:func:`H2K_futex_cancel()` removes a thread that is blocked on a futex from
the futex hash table.  The caller must hold the BKL.

Functionality
~~~~~~~~~~~~~

We compute the hash bucket from the thread's `futex_ptr` field, remove the
thread from that bucket's ring with :c:func:`H2K_ring_remove()`, and set the
thread's `r00` field to -1.

H2K_futex_init
--------------

.. c:function:: void H2K_futex_init(void)

Description
~~~~~~~~~~~

:c:func:`H2K_futex_init()` initializes the futex data structures.

Functionality
~~~~~~~~~~~~~

In testing builds, H2K_futexhash is set to NULL for each element in the
array.  Production builds require no runtime work.

