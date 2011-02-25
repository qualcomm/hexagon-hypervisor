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

Testing
-------

Tested cases
~~~~~~~~~~~~

* Call all traps from:
** User mode
** Guest mode

Harness
~~~~~~~

We link only with the vmtrap object file.
The harness will have a helper functions:

.. cfunction:: void TH_guest_trap()

This fucntion serves to purposes:

It sets the TH_saw_guest_tap = 1 to indicate that a guest permission error was caught. 

It ensures the stack is in the guest stack location if TH_expected_guest_stack == 1 or
that the stack is not in the guest stack location if TH_expected_guest_stack == 0. 
 


