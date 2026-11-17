:mod:`bzero` -- Byte-range zeroing
====================================

.. module:: bzero

H2K_bzero
---------

.. c:function:: void H2K_bzero(void *dst, u32_t size)

	:param dst: Start of the memory range to clear
	:param size: Number of bytes to clear

Writes zero to each of the ``size`` bytes beginning at ``dst``.  A size of zero
performs no writes.
