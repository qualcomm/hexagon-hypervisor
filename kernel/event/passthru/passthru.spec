:mod:`passthru` -- call a passthru interrupt handler
====================================================

.. module:: passthru


H2K_passthru
------------

.. cfunction:: H2K_passthru(u32_t phys_int, H2K_thread_context *unused1, u32_t unused2)

	:param phys_int: Physical interrupt number
	:param unused1: Context for the current thread
	:param unused2: Current hardware thread number

Description
~~~~~~~~~~~

Posts the virtual interrupt registered for the given physical interrupt.

Functionality
~~~~~~~~~~~~~

Looks up the VM ID and virtual-CPU ID in the global inthandlers array, as
registered by :cfunc:`H2K_register_passthru()`. Finds the corresponding
vmblock and CPU context (for per-CPU virtual interrupts). Calls the intop
handler chain via vmblock->intinfo to post the virtual interrupt.

Testing
-------

TBD.
