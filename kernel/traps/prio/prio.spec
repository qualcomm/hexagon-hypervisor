
:mod:`prio` -- managing priorities
===================================

.. module:: prio

Priority Management Trap Handlers


H2K_prio_get
------------

.. cfunction:: u32_t H2K_prio_get(H2K_thread_context *me)

Description
~~~~~~~~~~~

Gets the priority for the current thread context.

Input
~~~~~

Argument 0: Pointer to the current thread context

Output
~~~~~~

Returns the priority

Functionality
~~~~~~~~~~~~~

Reads the priority field out of the thread context and returns the resulting value.

H2K_prio_set
------------

.. cfunction:: u32_t H2K_prio_set(H2K_thread_context *dest, u32_t prio, H2K_thread_context *me)


Description
~~~~~~~~~~~

UNIMPLEMENTED.  Returns -1.

Input
~~~~~

Argument 0: Destination thread
Argument 1: new priority
Argument 2: Pointer to the current thread context

Output
~~~~~~

Returns -1.


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
H2K_prio_get returns the correct priority.

We also check that H2K_prio_set returns -1.



