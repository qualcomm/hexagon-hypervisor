:mod:`dma` -- handle DMA exceptions
====================================

.. module:: dma

H2K_handle_dma
--------------

.. c:function:: H2K_handle_dma()

Description
~~~~~~~~~~~

Handles a DMA TLB-miss exception on arch V73 and later.

Functionality
~~~~~~~~~~~~~

The handler saves the interrupted context and verifies that the exception
cause is a DMA TLB miss (cause ``0x76`` or ``0x77``).  If the cause is not a
DMA TLB miss, control is transferred to :c:func:`H2K_error_nonxe()`.

When ``COUNT_TLB_EVENTS`` is enabled, the handler atomically increments the
current VM's DMA-TLB-miss counter.  It then places ``badva`` in the register
expected by :c:func:`H2K_tlbmiss_common()` and transfers control there for
normal TLB-miss handling.

The handler is compiled only for ``ARCHV >= 73``.
