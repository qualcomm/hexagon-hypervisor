
ASM_REF_CODE(Bootup code is difficult to write in C)

:mod:`boot` -- startup routines
===============================

.. module:: boot

H2_init
-------

.. c:function:: void h2_init()

This function starts the kernel, if it has not been started already.

Functionality
~~~~~~~~~~~~~

This function merely returns. However, it must be called from main from the
boot thread.  This forces the linker to include the object in the final
executable.

This file references H2K_event_vectors, which references H2K_handle_trap0.
Together these should pull in the remainder of required files.

start
-----

.. c:function:: start()

This routine boots the machine.

Functionality
~~~~~~~~~~~~~

We assume that the machine is entirely off.

We initialize caches and the TLB, and set up the stack pointer.

Finally, we call :c:func:`H2K_thread_boot()`.

H2K_handle_reset
----------------

.. c:function:: H2K_handle_reset()

This routine boots a new processor, or after a soft reset

Functionality
~~~~~~~~~~~~~

We set up kernel stack pointer, and then call :c:func:`H2K_switch()` with both
arguments set to NULL.


Testing
-------


Samples
~~~~~~~

Machine state is set up correctly at boot.

Important Cases
~~~~~~~~~~~~~~~

* Check that other threads have booted
* Check that syscfg is configured correctly

Harness
~~~~~~~

Simple normal H2 application will call h2_init, and then check that system
registers have expected values.



