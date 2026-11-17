:mod:`thread_id` -- thread identifiers and lookup
====================================================

.. module:: thread_id

An ``H2K_id_t`` is a 32-bit identifier containing a reserved virtual-interrupt
field, a VM-local CPU index, and a VM index.  The VM index selects an entry in
``H2K_gp->vmblocks``; the CPU index selects a context in that VM's context
array.

H2K_thread_id
-------------

.. c:function:: H2K_id_t H2K_thread_id(H2K_thread_context *me)

	:param me: Thread context
	:returns: The ID stored in ``me``

Description
~~~~~~~~~~~

Returns the identifier belonging to a thread context.


H2K_id_cpuidx_to_context
------------------------

.. c:function:: static inline H2K_thread_context *H2K_id_cpuidx_to_context(struct H2K_vmblock_struct *vmblock, u32_t cpuidx)

	:param vmblock: VM block containing the thread contexts
	:param cpuidx: VM-local CPU index
	:returns: The selected context, or NULL when ``cpuidx`` is outside the VM's CPU range

Description
~~~~~~~~~~~

Converts a VM-local CPU index into its thread-context pointer.


H2K_id_to_context
-----------------

.. c:function:: static inline H2K_thread_context *H2K_id_to_context(H2K_id_t id)

	:param id: Thread identifier
	:returns: The corresponding thread context, or NULL when the VM does not exist or the CPU index is invalid

Description
~~~~~~~~~~~

Looks up the VM using ``id.vmidx`` and then calls
:c:func:`H2K_id_cpuidx_to_context()` with ``id.cpuidx``.


H2K_id_from_context
-------------------

.. c:function:: static inline H2K_id_t H2K_id_from_context(H2K_thread_context *me)

	:param me: Thread context
	:returns: The ID stored in ``me``

Description
~~~~~~~~~~~

Returns ``me->id`` without performing a lookup.
