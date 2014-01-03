:mod:`vmtrap` -- Virtual Machine Trap Handler
=============================================

.. module:: vmtrap


H2K_handle_trap1
----------------

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

H2K_vmtraptab
-------------

.. cfunction:: H2K_vmtraptab()

Description
~~~~~~~~~~~

The vmtraptab is similar to :cfunc:`H2K_traptab()`, as it is a table of destinations.
However, only jumps are used, all vmtrap functions take a single argument of the thread
context.

Functionality
~~~~~~~~~~~~~

The table is as follows, and matches the Hexagon VM Architecture Spec:

 - 0x00	:cfunc:`H2K_vmtrap_version()`: Virtual Machine Architecture Version
 - 0x01	:cfunc:`H2K_vmtrap_return()`: VM Return
 - 0x02	:cfunc:`H2K_vmtrap_setvec()`: Set Event Vector Table Address
 - 0x03	:cfunc:`H2K_vmtrap_setie()`: Set Interrupt Enabled Status
 - 0x04	:cfunc:`H2K_vmtrap_getie()`: Get Interrupt Enabled Status
 - 0x05	:cfunc:`H2K_vmtrap_intop()`: Virtual Interrupt Controller Operation
 - 0x06	:cfunc:`H2K_vmtrap_bad()`: Invalid Virtual Instruction
 - 0x07	:cfunc:`H2K_vmtrap_bad()`: Invalid Virtual Instruction
 - 0x08	:cfunc:`H2K_vmtrap_bad()`: Invalid Virtual Instruction
 - 0x09	:cfunc:`H2K_vmtrap_bad()`: Invalid Virtual Instruction
 - 0x0A	:cfunc:`H2K_vmtrap_clrmap()`: Clear Cache of Address Translations
 - 0x0B	:cfunc:`H2K_vmtrap_newmap()`: Register New Address Translations
 - 0x0C	:cfunc:`H2K_vmtrap_bad()`: Invalid Virtual Instruction
 - 0x0D	:cfunc:`H2K_vmtrap_cachectl()`: Request manipulation of caches
 - 0x0E	:cfunc:`H2K_vmtrap_get_pcycles()`: Get virtual processor cycles
 - 0x0F	:cfunc:`H2K_vmtrap_set_pcycles()`: Set virtual processor cycles
 - 0x10	:cfunc:`H2K_vmtrap_wait()`: Wait for VM Event
 - 0x11	:cfunc:`H2K_vmtrap_yield()`: Yield to other Virtual Processor at same priority
 - 0x12	:cfunc:`H2K_vmtrap_start()`: Start a new Virtual Processor
 - 0x13	:cfunc:`H2K_vmtrap_stop()`: Shut down this virtual processor
 - 0x14	:cfunc:`H2K_vmtrap_vmpid()`: Get virtual processor ID
 - 0x15	:cfunc:`H2K_vmtrap_setregs()`: Set Guest Registers
 - 0x16	:cfunc:`H2K_vmtrap_getregs()`: Get Guest Registers
 - 0x17	:cfunc:`H2K_vmtrap_bad()`: Invalid Virtual Instruction
 - 0x18	:cfunc:`H2K_vmtrap_timer()`: Operate Virtual Timer
 - 0x19	:cfunc:`H2K_vmtrap_pmuctrl()`: Virtual PMU control
 - 0x1A	:cfunc:`H2K_vmtrap_info()`: System configuration info
 - 0x1B	:cfunc:`H2K_vmtrap_bad()`: Invalid Virtual Instruction
 - 0x1C	:cfunc:`H2K_vmtrap_bad()`: Invalid Virtual Instruction
 - 0x1D	:cfunc:`H2K_vmtrap_bad()`: Invalid Virtual Instruction
 - 0x1E	:cfunc:`H2K_vmtrap_bad()`: Invalid Virtual Instruction
 - 0x1F	:cfunc:`H2K_vmtrap_bad()`: Invalid Virtual Instruction


H2K_vmtrap_bad
--------------

.. cfunction:: H2K_vmtrap_bad()

Description
~~~~~~~~~~~

This routine handles a bad VM trap

Functionality
~~~~~~~~~~~~~

We use :cfunc:`H2K_vm_event()` to raise an error in the guest.


Testing
-------

* Call all traps from:

  * User mode
  * Guest mode

Harness
~~~~~~~

We link only with the vmtrap object file.
The harness will have a helper functions:

.. cfunction:: void TH_guest_trap()

This fucntion serves to purposes:

It sets the TH_saw_guest_tap = 1 to indicate that a guest permission error was caught. 

It ensures the stack is in the guest stack location if TH_expected_guest_stack == 1 or
that the stack is not in the guest stack location if TH_expected_guest_stack == 0. 
 


