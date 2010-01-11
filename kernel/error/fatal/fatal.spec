
:mod:`fatal` -- errors that can not be corrected
=================================================

.. module:: fatal

H2K_fatal_kernel
----------------

.. cfunction:: void H2K_fatal_kernel(s16_t error_id, thread_context *me, u32_t info0, u32_t info1, u32_t hthread)

Description
~~~~~~~~~~~

H2K_fatal_kernel handles a fatal kernel error.  The kernel will attempt to
halt execution completely, and calls the function specified by 
H2K_fatal_kernel_handler.

Input
~~~~~

Argument 0: The ID of the fatal kernel error
Argument 1: The context of the currently running thread
Argument 2: The hardware thread number

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

First, we log the error.

Next, we call H2K_fatal_kernel_handler.

H2K_fatal_thread
----------------

.. cfunction:: void H2K_fatal_thread(s16_t error_id, H2K_thread_context *me, u32_t info0, u32_t info1, u32_t hthread)

Description
~~~~~~~~~~~

H2K_fatal_thread handles a fatal thread error.  A fatal thread error is an error
from a thread that the thread cannot handle.  For example, if the thread event vector
is NULL, the thread is incapable of recovering from an error.  

When a fatal thread error occurs, the thread will be terminated and the error
will be logged.

Input
~~~~~

Argument 0: The ID of the fatal thread error
Argument 1: The context of the currently running thread
Argument 2: The hardware thread number

Functionality
~~~~~~~~~~~~~

We log the error, and then sibcall to H2K_thread_stop.


