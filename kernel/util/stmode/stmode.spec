

:mod:`stmode` -- Single Thread Mode Assistance
==============================================

.. module:: stmode

This module contains assist functions for entering and leaving Single Thread
Mode, where only one thread is active.  This mode is required for certain 
core reconfiguration services.

H2K_stmode_begin
----------------

.. c:function:: s32_t H2K_stmode_begin()

	:returns: 0 on success, nonzero on failure

Description
~~~~~~~~~~~

This function attempts to enter single thread mode.  If single thread mode
cannot be entered, a nonzero value is returned.  Single thread mode can only
be entered when no other threads or interrupts are being processed.

Functionality
~~~~~~~~~~~~~

The BKL is acquired.  We then globally disable interrupts.

If the MODECTL register indicates that only one thread is active, then we free
the BKL and return success.

Otherwise, we reenable interrupts globally, free the BKL, and return failure.


H2K_stmode_end
--------------

.. c:function:: void H2K_stmode_end()


Description
~~~~~~~~~~~

This function leaves single thread mode.

Functionality
~~~~~~~~~~~~~

Reenables global interrupts and returns.


Testing
-------

Samples
~~~~~~~

* I/O: SYSCFG.G
* Input: number of running threads
* I/O: BKL should be acquired & freed

Important cases
~~~~~~~~~~~~~~~

* Only one thread on: H2K_stmode_begin should succeed
* One thread on, at least one thread waiting: H2K_stmode_begin should succeed
* One thread on, at least one thread running: H2K_stmode_begin should fail

Harness
~~~~~~~

Configure SYSCFG, number of running/waiting threads appropriately.  Then call
H2K_stmode_begin() and check that the expected return value was returned.  Also
check SYSCFG.G bit is set to the expected value.

Check that H2K_stmode_end sets the SYSCFG.G bit to the expected value.



