
:mod:`setup` -- set up initial kernel state
============================================

.. module:: setup

init_setup
----------

.. cfunction:: void init_setup()

Description
~~~~~~~~~~~

Sets up the kernel at boot time.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

The init_setup function initializes kernel state at boot time.

This function calls other initialization functions:

	ready_init()
	runlist_init()

Additionally, it ???


H2K_thread_boot
---------------

.. cfunction:: void H2K_thread_boot()

Initializes the core and sets up the boot thread.  This function is jumped to
by the boot code.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

First, we call H2K_init_setup to intialize the kernel data structures.

Next, we set up the boot thread context.

Once we have set up the boot thread context and placed it into the runlist, we
switch to the thread.  This will cause the kernel to go to the continuation
function, which we have initialized to H2K_interrupt_restore(), and will end up 
at qdsp6_pre_main, in crt0.

