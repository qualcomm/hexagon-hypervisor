
:mod:`futex_common` -- Futex Common Routines
============================================

.. module:: futex_common

H2K_futex_hash_add
------------------

.. c:function:: void H2K_futex_hash_add_ring(H2K_thread_context **ring, H2K_thread_context *me)

Description
~~~~~~~~~~~

:c:func:`H2K_futex_hash_add_ring()` adds the thread context to the ring

Functionality
~~~~~~~~~~~~~

Put in the hash at the right place (priority sorted)

