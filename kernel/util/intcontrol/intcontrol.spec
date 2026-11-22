:mod:`intcontrol` -- Interrupt-controller access
===================================================

.. module:: intcontrol

The helpers select the processor's L1 interrupt instructions for interrupt
numbers below 32 and the mapped L2 interrupt-controller registers for larger
interrupt numbers.  The caller supplies a valid interrupt number for the
configured hardware.


H2K_intcontrol_enable
---------------------

.. c:function:: static inline void H2K_intcontrol_enable(u32_t intno)

	:param intno: Interrupt number to enable

For ``intno < 32``, executes ``ciad`` for the corresponding bit.  Otherwise,
writes the bit to the L2 acknowledgement/enable register selected by
``H2K_gp->l2_ack_base``.


H2K_intcontrol_disable
----------------------

.. c:function:: static inline void H2K_intcontrol_disable(u32_t intno)

	:param intno: Interrupt number to disable

For ``intno < 32``, executes ``siad`` for the corresponding bit.  Otherwise,
writes the bit to the L2 interrupt-mask register at the configured L2 base.


H2K_intcontrol_raise
--------------------

.. c:function:: static inline void H2K_intcontrol_raise(u32_t intno)

	:param intno: Interrupt number to raise

For ``intno < 32``, executes ``swi`` for the corresponding bit.  Otherwise,
writes the bit to the L2 software-interrupt register at the configured L2 base.
