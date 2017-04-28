
:mod:`info` -- Get system configuration info
=============================================


H2K_info
--------

.. c:function:: void H2K_info(u32_t op, H2K_thread_context *me)

	:param op: Configuration query
	:param me: Pointer to the context for the current thread

Description
~~~~~~~~~~~

Return the requested configuration parameter

Functionality
~~~~~~~~~~~~~

Return one of: build ID, boot flags, STLB configuration, SYSCFG register, REV register.
Return -1 on unknown request.
