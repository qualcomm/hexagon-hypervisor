
:mod:`setup` -- set up initial kernel state
============================================

.. module:: setup

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

