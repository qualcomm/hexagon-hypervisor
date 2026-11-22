:mod:`vmipi` -- Virtual machine IPI management
================================================

.. module:: vmipi

``H2K_gp->mask_for_ipi`` records the hardware threads with a pending VM IPI.
The mask has the opposite sense from the hardware assignment register: a set
bit in ``mask_for_ipi`` means that the corresponding hardware thread receives
the IPI, so the value written to ``VM_IPI_INT`` is the complement of the mask.


H2K_vm_ipi_do
-------------

.. c:function:: void H2K_vm_ipi_do(u32_t ipi_intno, H2K_thread_context *me, u32_t hwtnum) IN_SECTION(".text.vm.ipi")

	:param ipi_intno: Dedicated IPI interrupt number
	:param me: Current VCPU context, or ``NULL`` when there is no VCPU work to process
	:param hwtnum: Hardware-thread number whose IPI bit was received

Handles the dedicated VM IPI interrupt.  It clears ``hwtnum`` in
``H2K_gp->mask_for_ipi`` and writes the complement of the updated mask to
``VM_IPI_INT``.  If other hardware-thread bits remain set, it raises the IPI
again so those threads can handle their pending requests.  When ``me`` is not
``NULL``, it runs :c:func:`H2K_vm_do_work_withlock()` while holding the BKL.
With ``DO_PROFILE`` enabled, it also records a sample for ``hwtnum``.


H2K_vm_ipi_send_withlock_mask
-----------------------------

.. c:function:: s32_t H2K_vm_ipi_send_withlock_mask(u32_t mask) IN_SECTION(".text.vm.ipi")

	:param mask: Hardware-thread mask to add to ``H2K_gp->mask_for_ipi``
	:returns: 0

The caller must hold the BKL.  Atomically ORs ``mask`` into
``H2K_gp->mask_for_ipi``, writes the complement of the resulting mask to
``VM_IPI_INT``, and raises the IPI interrupt.  The BKL remains held on return.


H2K_vm_ipi_send_withlock
------------------------

.. c:function:: s32_t H2K_vm_ipi_send_withlock(H2K_thread_context *dest) IN_SECTION(".text.vm.ipi")

	:param dest: Destination VCPU context
	:returns: 0, or the result of :c:func:`H2K_vm_ipi_send_withlock_mask()`

If ``dest`` is running, builds a mask containing ``dest->hthread`` and calls
:c:func:`H2K_vm_ipi_send_withlock_mask()`.  If ``dest`` is not running, it does
nothing and returns zero.  The caller must hold the BKL.


H2K_vm_ipi_send_mask
--------------------

.. c:function:: void H2K_vm_ipi_send_mask(u32_t mask) IN_SECTION(".text.vm.ipi")

	:param mask: Hardware-thread mask to receive the IPI

Same as :c:func:`H2K_vm_ipi_send_withlock_mask()`, except that this function
acquires the BKL before the operation and releases it afterward.


H2K_vm_ipi_send
---------------

.. c:function:: void H2K_vm_ipi_send(H2K_thread_context *dest) IN_SECTION(".text.vm.ipi")

	:param dest: Destination VCPU context

If ``dest`` is running, builds a mask containing ``dest->hthread`` and calls
:c:func:`H2K_vm_ipi_send_mask()`.  If ``dest`` is not running, it does nothing.
This is the unlocked wrapper around the same mask-based send operation used by
:c:func:`H2K_vm_ipi_send_mask()`.
