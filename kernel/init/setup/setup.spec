
MODULE: setup


FUNCTION: void init_setup()

DESCRIPTION:

Sets up the kernel at boot time.

INPUTS:

OUTPUTS:

BEHAVIOR:

The init_setup function initializes kernel state at boot time.

This function calls other initialization functions:

	ready_init()
	runlist_init()

Additionally, it ???

