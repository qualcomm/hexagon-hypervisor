
:mod:`setup` -- set up initial kernel state
===========================================

.. module:: setup

H2K_init_setup
--------------

.. c:function:: void H2K_init_setup()

Description
~~~~~~~~~~~

Sets up the kernel at boot time.

Functionality
~~~~~~~~~~~~~

The :c:func:`H2K_init_setup()` function initializes kernel state at boot time.

This function calls other initialization functions:

* :c:func:`H2K_ready_init()`
* :c:func:`H2K_runlist_init()`
* :c:func:`H2K_lowprio_init()`
* :c:func:`H2K_futex_init()`
* :c:func:`H2K_intconfig_init()`
* :c:func:`H2K_thread_init()`


H2K_thread_boot
---------------

.. c:function:: void H2K_thread_boot()

Initializes the core and sets up the boot thread.  This function is jumped to
by the boot code.

Functionality
~~~~~~~~~~~~~

First, we call :c:func:`H2K_init_setup()` to intialize the kernel data structures.

Next, we set up the boot thread context.  Specifically:
* ELR should point to qdsp6_pre_main
* Continuation should point to :c:func:`H2K_interrupt_restore()`
* Trapmask should be initialized to all 1's.

Once we have set up the boot thread context and placed it into the runlist, we
switch to the thread.  This will cause the kernel to go to the continuation
function, and will end up at qdsp6_pre_main, in crt0.

Testing
-------


Samples
~~~~~~~

* Init routines must be called
* We must call :c:func:`H2K_switch()` to boot thread
* Boot thread fields should be set correctly

Important Cases
~~~~~~~~~~~~~~~

* No inputs to functions, just check correct state is set up.

Harness
~~~~~~~

Standalone application which defines all the initialization functions,
as well as :c:func:`H2K_switch()`.




