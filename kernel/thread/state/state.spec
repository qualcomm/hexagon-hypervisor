
:mod:`thread_state` -- Read thread context
==========================================

.. module:: thread_state

H2K_thread_state
----------------

.. c:function:: u64_t H2K_thread_state(H2K_id_t id, u32_t offset, H2K_thread_context *me)

	:param id: Thread to query
	:param offset: Offset to read in context
	:param me: Pointer to the current context
	:returns: 64-bit value at offset, or -1 on error

Description
~~~~~~~~~~~

Read the value from given thread/offset.

Functionality
~~~~~~~~~~~~~

Find the thread context from the given id.  Read 8 bytes starting at the given
offset (unaligned ok).  Return error if attempting to read from a different VM,
or if CPU index is outside range of configured CPUs for this VM, or if offset
is larger than thread context.
