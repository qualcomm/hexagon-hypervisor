:mod:`error` -- Handle an error exception
======================================

.. module:: error

H2K_handle_error
----------------

.. cfunction:: H2K_handle_error()

Description
~~~~~~~~~~~

XXX: not completely specd/implemented

H2K_handle_error takes an error and applies the appropriate handling method.

Input
~~~~~

Output
~~~~~~

Description
~~~~~~~~~~~

NOTE: For V2, we will not be able to safely do the Double Exception check.

First, we lock the kernel, so we can safely use a global regsiter for scratch.

We then copy a register into the scratch register, so we can copy SSR into 
a general register.  We mask off the CAUSE field and check to see whether 
the CAUSE is "Double Exception".  If so, we vector to code that sets up
a H2K_fatal_kernel.

Otherwise, we copy the global scratch register back, unlock the kernel, and can
safely swap SGP with a normal register to save state.

After saving sufficient state, we check to see whether the error handler is 
NULL.  If the error handler is NULL, we call H2K_fatal_thread.  If the 
error was caused by an error handler, we call H2K_fatal_thread.  

Otherwise, if the thread was in User mode, we swap GOSP with R29. We also save
ELR in GELR, BADVA in GBADVA, and cause in GCAUSE.  We set ELR to the event 
handler address, restore registers, and return.

TBD: counter for consecutive fatal errors?

