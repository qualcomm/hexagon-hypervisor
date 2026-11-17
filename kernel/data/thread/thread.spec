
:mod:`thread` -- data structures for threads
============================================

.. module:: thread

This module contains the data structures for threads.

H2K_thread_context_clear
------------------------

.. c:function:: void H2K_thread_context_clear(H2K_thread_context *thread)

	:param thread: pointer to a thread context

Description
~~~~~~~~~~~

Zeros a thread context.

Functionality
~~~~~~~~~~~~~

Sets every bit in the thread context to zero.

H2K_ext_context_clear
---------------------

.. c:function:: void H2K_ext_context_clear(H2K_ext_context *ext)

	:param ext: pointer to an extended thread context

Description
~~~~~~~~~~~

Zeros an extended thread context when ``DO_EXT_SWITCH`` is enabled.

Functionality
~~~~~~~~~~~~~

Treats the context as an array of 64-bit values and sets every value to zero.

H2K_thread_init
---------------

.. c:function:: void H2K_thread_init(void)

Description
~~~~~~~~~~~

Initializes thread-related data structures.

Functionality
~~~~~~~~~~~~~
Empty function body.

