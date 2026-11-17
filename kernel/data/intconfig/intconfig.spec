
:mod:`intconfig` -- configuration for interrupt handling
========================================================

.. module:: intconfig

H2K_intconfig_init
------------------

.. c:function:: void H2K_intconfig_init(u32_t ssbase)

	:param ssbase: Subsystem private-register base used to select the L2 interrupt configuration

Description
~~~~~~~~~~~

H2K_intconfig_init initializes the interrupt configuration data.

Functionality
~~~~~~~~~~~~~

All handler pointers are initialized to NULL.  The reschedule,
cluster-reschedule, VM IPI, and timer interrupt handlers are initialized to
:c:func:`H2K_resched()`, :c:func:`H2K_resched_cluster()`,
:c:func:`H2K_vm_ipi_do()`, and :c:func:`H2K_timer_int()`, respectively.  The
function then calls :c:func:`H2K_intconfig_l2_init()` to initialize the L2
interrupt controller when H2 controls that controller and devices are enabled.


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


H2K_intconfig_l2_init
---------------------

.. c:function:: static void H2K_intconfig_l2_init(u32_t ssbase)

	:param ssbase: Subsystem base used to select MSS or LPASS interrupts

Description
~~~~~~~~~~~

Initializes the L2 interrupt controller when H2 controls it and devices are
enabled.  Otherwise, this function does nothing.

Functionality
~~~~~~~~~~~~~

Disables and clears all L2 interrupts, configures them as rising-edge triggered,
and changes the required SPI, TLMM, SPMI, and LPASS USB interrupts to level-high
triggering.  ``ssbase`` selects the MSS or LPASS (default is LPASS, if MSS is not specified)
interrupt numbers on V5 and later.  Finally, the function enables L2 interrupts.


H2K_intconfig_l2vic_crash
-------------------------

.. c:function:: void H2K_intconfig_l2vic_crash()

Description
~~~~~~~~~~~

Saves L2 interrupt-controller state for crash diagnostics when ``CRASH_DEBUG``
is enabled.

Functionality
~~~~~~~~~~~~~

Copies each L2 controller word's enabled and pending registers into
``H2K_gp->crash_l2vic_enabled`` and ``H2K_gp->crash_l2vic_pending``.


Testing
-------

For H2K_intconfig_init, we check to make sure that the array has been
initialized after the call.

