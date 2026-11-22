:mod:`vmop` -- Virtual machine operations
============================================

.. module:: vmop

The VM operation interface dispatches VM lifecycle requests through a table of
operation handlers.  Operation results are returned as signed 32-bit values;
``-1`` indicates an invalid operation, invalid target, or rejected request.


H2K_vmop
--------

.. c:function:: s32_t H2K_vmop(vmop_t op, u32_t val1, u32_t val2, u32_t val3, u32_t val4, u32_t val5, H2K_thread_context *me) IN_SECTION(".text.misc.vmboot")

	:param op: One of ``VMOP_BOOT``, ``VMOP_STATUS``, ``VMOP_FREE``, ``VMOP_KILL_THREAD``, or ``VMOP_KILL_VM``
	:param val1: First operation-specific value
	:param val2: Second operation-specific value
	:param val3: Third operation-specific value
	:param val4: Fourth operation-specific value
	:param val5: Fifth operation-specific value
	:param me: Current thread context
	:returns: Result from the selected operation, or ``-1`` when ``op`` is invalid

Dispatches to the handler selected by ``op``.  The dispatcher passes zero as the
handler's unused operation argument and forwards ``val1`` through ``val5`` and
``me`` unchanged.


H2K_vmop_boot
-------------

.. c:function:: s32_t H2K_vmop_boot(vmop_t unused0, u32_t pc, u32_t sp, u32_t arg1, u32_t prio, u32_t vm, H2K_thread_context *me) IN_SECTION(".text.misc.vmboot")

	:param unused0: Unused operation argument
	:param pc: Initial program counter for the first VCPU
	:param sp: Initial stack pointer for the first VCPU
	:param arg1: Initial argument passed to the first VCPU
	:param prio: Priority of the first VCPU
	:param vm: Target VM index
	:param me: Current thread context; may be ``NULL``
	:returns: New thread ID from :c:func:`H2K_thread_create_no_squash()`, or ``-1`` on failure

Boots a previously allocated VM by creating its first VCPU.  The target VM must
exist, must be a child of the caller when ``me`` is non-``NULL``, and must have
no running VCPUs.  The remaining arguments are passed to
:c:func:`H2K_thread_create_no_squash()` together with the target VM block.


H2K_vmop_status
---------------

.. c:function:: s32_t H2K_vmop_status(vmop_t unused0, u32_t op, u32_t vm, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me) IN_SECTION(".text.misc.vmboot")

	:param unused0: Unused operation argument
	:param op: One of ``VMOP_STATUS_STATUS`` or ``VMOP_STATUS_CPUS``
	:param vm: Target VM index, or ``VMOP_STATUS_VMIDX_SELF`` for the caller's VM
	:param unused3: Unused operation argument
	:param unused4: Unused operation argument
	:param unused5: Unused operation argument
	:param me: Current thread context
	:returns: VM status or running-VCPU count, or ``-1`` on error

Returns the requested status for the caller's VM or a child VM.  The operation
holds the BKL while reading the VM block:

* ``VMOP_STATUS_STATUS`` returns ``vmblock->status``.
* ``VMOP_STATUS_CPUS`` returns ``vmblock->num_cpus``.

A null caller, invalid or missing VM, unrelated VM, or unsupported status
operation returns ``-1``.


H2K_vmop_free
-------------

.. c:function:: s32_t H2K_vmop_free(vmop_t unused0, u32_t vm, u32_t unused2, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me) IN_SECTION(".text.misc.vmboot")

	:param unused0: Unused operation argument
	:param vm: Target child VM index
	:param unused2: Unused operation argument
	:param unused3: Unused operation argument
	:param unused4: Unused operation argument
	:param unused5: Unused operation argument
	:param me: Current thread context; may be ``NULL``
	:returns: 0 on success, or ``-1`` on error

Frees the target VM block and clears its global VM-block pointer.  The target
must exist, must be a child of the caller when ``me`` is non-``NULL``, and must
have no running VCPUs.  A running target, invalid target, missing target, or
unauthorized caller returns ``-1``.


kill_thread_locked
------------------

.. c:function:: static void kill_thread_locked(H2K_vmblock_t *vmblock, H2K_thread_context *target)

	:param vmblock: VM block containing ``target``
	:param target: VCPU context to terminate

Marks a live target with ``H2K_VMSTATUS_KILL`` and
``H2K_VMSTATUS_VMWORK`` while the caller holds the BKL.  A waiting VCPU is
removed from the waiting mask and made ready; a running VCPU receives a VM IPI;
an interrupt-blocked or futex-blocked VCPU is cancelled from that state and
made ready; and a ready VCPU requires no additional state transition.  Dead
VCPUs are ignored.


H2K_vmop_kill_thread
--------------------

.. c:function:: s32_t H2K_vmop_kill_thread(vmop_t unused0, u32_t id, u32_t unused2, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me) IN_SECTION(".text.misc.vmboot")

	:param unused0: Unused operation argument
	:param id: Raw ``H2K_id_t`` identifying the target VM and VCPU
	:param unused2: Unused operation argument
	:param unused3: Unused operation argument
	:param unused4: Unused operation argument
	:param unused5: Unused operation argument
	:param me: Current thread context
	:returns: 0 on success, or ``-1`` for an invalid or unauthorized target

Requests termination of the identified VCPU by calling
:c:func:`kill_thread_locked()` under the BKL.  The caller may target its own VM
or a child VM.  If the target VM has already been freed, the operation returns
0 so callers can perform cleanup without a separate existence check.  A null
caller, unauthorized VM, or invalid VCPU context returns ``-1``.


H2K_vmop_kill_vm
----------------

.. c:function:: s32_t H2K_vmop_kill_vm(vmop_t unused0, u32_t vm, u32_t status, u32_t unused3, u32_t unused4, u32_t unused5, H2K_thread_context *me) IN_SECTION(".text.misc.vmboot")

	:param unused0: Unused operation argument
	:param vm: Target VM index, or ``VMOP_KILL_VM_SELF`` for the caller's VM
	:param status: Exit status to store in the target VM block
	:param unused3: Unused operation argument
	:param unused4: Unused operation argument
	:param unused5: Unused operation argument
	:param me: Current thread context
	:returns: 0 on success, or ``-1`` for an invalid or unauthorized target

Marks every VCPU in the target VM for termination using
:c:func:`kill_thread_locked()`.  The target may be the caller's VM or a child
VM.  Before marking the VCPUs, stores ``status`` in the VM block so that the
termination path reports that value.  A missing non-self VM is treated as
already terminated and returns 0.  A null caller, invalid target index, or
unrelated VM returns ``-1``.
