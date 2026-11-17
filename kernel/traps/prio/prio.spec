:mod:`prio` -- Calling-thread priority management
==================================================

.. module:: prio

The priority traps operate only on the calling thread.  They do not accept or
resolve a destination thread ID.

H2K_prio_get
------------

.. c:function:: u32_t H2K_prio_get(H2K_thread_context *me)

	:param me: Calling thread context
	:returns: Calling thread's base priority

Description
~~~~~~~~~~~

Returns ``me->base_prio``.


H2K_prio_set
------------

.. c:function:: s32_t H2K_prio_set(u32_t prio, H2K_thread_context *me)

	:param prio: New base priority for the calling thread
	:param me: Calling thread context
	:returns: Previous base priority, or -1 for an invalid priority

Description
~~~~~~~~~~~

Sets the calling thread's base priority.  A priority outside the valid range,
or better than the VM's permitted ``bestprio``, is rejected.

Functionality
~~~~~~~~~~~~~

Updates ``base_prio``.  If the new base priority is better than the current
effective priority, the effective priority is improved immediately.  An
existing priority-inheritance boost that is better than the new base priority
is preserved.
