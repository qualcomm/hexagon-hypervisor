


:mod:`intcontrol` -- Interrupt Controller Manipulation
======================================================

.. module:: intcontrol

This unit contains code to interface with the interrupt control hardware.

H2K_intcontrol_enable
---------------------

.. cfunction:: void H2K_intcontrol_enable(u32_t intno)

	:param intno: the interrupt to enable


Description
~~~~~~~~~~~

:cfunc:`H2K_intcontrol_enable()` enables an interrupt at the hardware interrupt controller.

INPUT_ASSERT((intno < MAX_INTERRUPTS) && ((ARCHV <= 3) || (intno != 31)))

.. InputAssert::
	assert((intno < MAX_INTERRUPTS) && ((ARCHV <= 3) || (intno != 31)))

Functionality
~~~~~~~~~~~~~

For V4 and later, we write the appropriate ENABLE_SET register of the L2 interrupt controller
for L2 interrupts.  For V3, and for L1 interrupts, we do a CIAD of the appropriate bit.

H2K_intcontrol_disable
----------------------
.. cfunction:: void H2K_intcontrol_disable(u32_t intno)

	:param intno: the interrupt to enable

Description
~~~~~~~~~~~

:cfunc:`H2K_intcontrol_disable()` disables  an interrupt at the hardware interrupt controller.

INPUT_ASSERT((intno < MAX_INTERRUPTS) && ((ARCHV <= 3) || (intno != 31)))

.. InputAssert::
	assert((intno < MAX_INTERRUPTS) && ((ARCHV <= 3) || (intno != 31)))

Functionality
~~~~~~~~~~~~~

For V4 and later, we write the appropriate ENABLE_SET register of the L2 interrupt controller
for L2 interrupts, and for L1 interrupts, we do a SIAD of the appropriate bit.  For V3, this 
function does nothing.


Testing
-------



