
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

XXX: UNIMPLEMENTED 

Description
~~~~~~~~~~~

Changes the priority for the specified thread ``dest`` to ``prio``.

Input
~~~~~

Argument 0: Destination thread
Argument 1: new priority
Argument 2: Pointer to the current thread context

Output
~~~~~~

Returns old priority?




