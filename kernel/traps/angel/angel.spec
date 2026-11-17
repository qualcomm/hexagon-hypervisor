ASM_REF_CODE(Angel trap handling is implemented in assembly)

:mod:`angel` -- Angel semihosting interface
============================================

.. module:: angel

H2K_trap_angel
--------------

.. c:function:: u32_t H2K_trap_angel(u32_t op, void *buf, u32_t arg)

	:param op: Angel system-call operation
	:param buf: Operation buffer or argument pointer
	:param arg: Operation argument
	:returns: Updated operation result values

Description
~~~~~~~~~~~

Sends an Angel semihosting request through the shared Angel communication
area.  With ``NULL_ANGEL_TRAP``, the routine returns immediately without
performing a request.

Functionality
~~~~~~~~~~~~~

For an enabled Angel trap, the handler serializes requests with the global
Angel lock, writes the operation arguments and current hardware-thread ID to
the core's Angel communication area, cleans the data cache, and signals the
transactor.  It polls until the transactor clears the completion flag, reads
back the result arguments, releases the lock, and returns.

The Angel operation constants, including ``SYS_EXIT``, ``SYS_WRITE``,
``SYS_READ``, and ``SYS_WRITECREG``, are defined in ``angel.h``.  The
communication area is shared with the simulator or external Angel transactor;
it is not a normal C function call interface.


H2K_angel_init
--------------

.. c:function:: static inline void H2K_angel_init(void)

Initializes the global Angel communication lock.
