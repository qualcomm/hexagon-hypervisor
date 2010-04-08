:mod:`vmtrap` -- Virtual Machine Trap Handler
=============================================

.. module:: vmtrap

.. cfunction:: void H2K_handle_trap1()


Description
~~~~~~~~~~~

This function handles the VM calls, which are implemented using the trap1
instruction.

If the thread is in user mode, we cause an error event.

If the hypercall is invalid, we cause an error event.

Otherwise, we call the corresponding hypercall.

Functionality
~~~~~~~~~~~~~

To facilitate C reference code for the actual trap calls, we will save off all
necessary registers.  These will be restored on the retrun.  Optimized
implementations may be able to save fewer registers.


