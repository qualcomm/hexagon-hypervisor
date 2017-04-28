
:mod:`thread_id` -- obtain unique ID
====================================

.. module:: thread_id

H2K_thread_id
-------------

.. c:function:: H2K_id_t H2K_thread_id(H2K_thread_context *me)

	:param me: Pointer to the current context
	:returns: A unique ID for the running thread

Description
~~~~~~~~~~~

The :c:func:`H2K_thread_id()` function returns a unique id for the requesting thread.

Functionality
~~~~~~~~~~~~~

Each thread should have a unique ID.

This helps us change priority of other threads.


H2K_id_to_context
-----------------

.. c:function:: inline H2K_thread_context *H2K_id_to_context(H2K_id_t id)

	:param id: Thread Identifier
	:returns: Pointer to the thread, NULL if ID is invalid

H2K_id_from_context
-------------------

.. c:function:: inline H2K_id_t H2K_id_from_context(H2K_thread_context *me)

	:param me: Pointer to the current context
	:returns: A unique ID for the running thread



Testing
-------


Samples
~~~~~~~

* Input: `me` 
* Output: unique ID

Important cases
~~~~~~~~~~~~~~~

* Various values of `me`

Harness
~~~~~~~

We link directly with the thread object file.

The test harness calls :c:func:`H2K_thread_id()` with `me`, and expects the return value
to be equal to the thread unique ID field.

