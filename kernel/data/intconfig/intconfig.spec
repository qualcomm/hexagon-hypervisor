
:mod:`intconfig` -- configuration for interrupt handling
========================================================

.. module:: intconfig

H2K_intconfig_init
------------------

.. cfunction:: void H2K_intconfig_init()

Description
~~~~~~~~~~~

H2K_intconfig_init initializes the interrupt configuration data and fast interrupt contexts

Functionality
~~~~~~~~~~~~~

All handler pointers are initialized to NULL.  The fastint mask is initialized 
to zeros.  The handler for reschedule is initialized to :cfunc:`H2K_resched()`.

Next, we initialize all fastint contexts to zeros.  We then update the hthread
fields to the correct hardware thread that will be using the fastint context, 
and the trapmask to exclude any trap may not be called by the fast interrupt
handler.



H2K_register_fastint
--------------------

.. cfunction:: void H2K_register_fastint(u32_t whatint, int (*fastint_handler)(u32_t x), H2K_thread_context *me)

	:param whatint: which interrupt to register
	:param fastint_handler: Address of the fast interrupt handler
	:param me: Context of the calling thread

Description
~~~~~~~~~~~

Modifies the interrupt configuration data to register or deregister a fast interrupt handler.

Functionality
~~~~~~~~~~~~~

When a valid function pointer is passed, the fastint_handler is placed in the
param field for the interrupt, and H2K_fastint is placed in the handler field.

When a NULL function pointer is passed, we deregister the associated interrupt.
We set the entries in the inthandlers array to NULL.  


H2K_register_passthru
---------------------

.. cfunction:: void H2K_register_passthru(u32_t phys_int, H2K_id_t id, u32_t virt_int)

	:param phys_int: Physical interrupt number
	:param id: Virtual CPU ID to receive interrupt (value 0 reserved)
	:param virt_int: Virtual interrupt number to post in receiving VM

Description
~~~~~~~~~~~

Modifies the interrupt configuration data to register or deregister a passthru interrupt.

Functionality
~~~~~~~~~~~~~

H2K_passthru is placed in the handler field for the physical interrupt, and the
ID is placed in the param field.  The reserved bits of the param field are
filled with the virtual interrupt number.  If the virtual interrupt number is
larger than the range accommodated by the reserved bits, the value 0 is used to
indicate that the mapping is stored in the vmblock.


Testing -------

For H2K_intconfig_init, we check to make sure that the array has been
initialized after the call.

For H2K_register_fastint, we call with a valid function pointer check and make
sure the fastint is registered correctly.  We then call with a NULL function
pointer and ensure the fastint is deregistered correctly.

Samples
~~~~~~~

* Input: fastint_handler
* Input: whatint
* Output: H2K_inthandlers[whatint].param
* Output: H2K_inthandlers[whatint].handler
* Output: IAD bit for interrupt whatint cleared

