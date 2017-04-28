
:mod:`prio` -- managing priorities
==================================

.. module:: prio

Priority Management Trap Handlers


H2K_prio_get
------------

.. c:function:: u32_t H2K_prio_get(H2K_thread_context *me)

	:param me: Pointer to the current thread context
	:returns: Returns the priority

Description
~~~~~~~~~~~

Gets the priority for the current thread context.

Functionality
~~~~~~~~~~~~~

Reads the priority field out of the thread context and returns the resulting value.

H2K_prio_set
------------

.. c:function:: u32_t H2K_prio_set(H2K_thread_context *dest, u32_t prio, H2K_thread_context *me)

	:param dest: Destination thread
	:param prio: new priority
	:param me: Pointer to the current thread context
	:returns: -1

Description
~~~~~~~~~~~

UNIMPLEMENTED.  Returns -1.


Testing
-------

Samples
~~~~~~~

* Priority of a thread

Important Cases
~~~~~~~~~~~~~~~

* All priorities

Harness
~~~~~~~

We only link with the library.

After setting the priority field in a thread context, we check that
:c:func:`H2K_prio_get()` returns the correct priority.

We also check that :c:func:`H2K_prio_set()` returns -1.



