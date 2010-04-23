
:mod:`angel` -- Angel Semihosting Interaction
=============================================

.. module:: angel

H2K_trap_angel
--------------

.. cfunction:: u64_t H2K_trap_angel(u32_t r0, u32_t r1)

	:param r0: Which angel trap
	:param r1: Information for the angel call

Description
~~~~~~~~~~~

The angel semihosting interface does not need to do anything for a standard
simulator, or for ANGEL requests over T32.

However, for using the ZeBu Angel Transactor, communication and synchronization
happens between special values in memory.


