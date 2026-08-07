
:mod:`intconfig` -- configuration for interrupt handling
========================================================

.. module:: intconfig

H2K_intconfig_init
------------------

.. c:function:: void H2K_intconfig_init()

Description
~~~~~~~~~~~

H2K_intconfig_init initializes the interrupt configuration data.

Functionality
~~~~~~~~~~~~~

All handler pointers are initialized to NULL.  The handler for reschedule
is initialized to :c:func:`H2K_resched()`.


H2K_register_passthru
---------------------

.. c:function:: void H2K_register_passthru(u32_t phys_int, H2K_id_t id, u32_t virt_int)

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

