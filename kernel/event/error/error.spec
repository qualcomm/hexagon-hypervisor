:mod:`error` -- Handle error and crash exceptions
=================================================

.. module:: error

H2K_handle_error
----------------

.. c:function:: H2K_handle_error()

Description
~~~~~~~~~~~

Handles an error exception and routes it to the appropriate VM event handling
path.

Functionality
~~~~~~~~~~~~~

Saves the interrupted context and extracts the exception cause.  When an
extended-context switch is active, an appropriate coprocessor exception may
restore the extended context.  Otherwise, the handler passes the bad address,
cause, and current thread to :c:func:`H2K_vm_event()`, reaches a preemption
point through :c:func:`H2K_preempt()` trying to take interrupt before returning,
and restores the context.


H2K_fatal_crash
---------------

.. c:function:: H2K_fatal_crash()

Description
~~~~~~~~~~~

Performs the requested software-crash actions and brings the system to a
controlled stop.

Functionality
~~~~~~~~~~~~~

The routine coordinates the crash sequence.  It can enter single-thread mode,
stop other hardware threads, release interrupted locks, save TLB state and L2
interrupt state when crash debugging is enabled, copy TCM state, clean and
invalidate caches, and execute the requested stop operation.


H2K_handle_nmi
---------------

.. c:function:: H2K_handle_nmi()

Description
~~~~~~~~~~~

Handles a non-maskable interrupt and routes it into crash handling or thread
termination.

Functionality
~~~~~~~~~~~~~

With crash debugging enabled, the routine saves the interrupted register and
special-register context, distinguishes soft and hard NMI, and joins the
common crash path.  For a hard NMI, only the selected hardware thread
continues while the others stop.  Without crash debugging, it saves the
context and transfers control to :c:func:`H2K_fatal_thread()`.


H2K_handle_debug
----------------

.. c:function:: H2K_handle_debug()

Description
~~~~~~~~~~~

Handles a debug exception.

Functionality
~~~~~~~~~~~~~

Saves the interrupted context, obtains the exception cause, and transfers
control to :c:func:`H2K_vm_event()` with the debug event-vector offset.
