:mod:`tlbmiss` -- TLB miss handling
===================================

.. module:: tlbmiss

The TLB miss handlers run with the TLB lock held and save the interrupted
context before calling the C TLB-fill path.

H2K_handle_tlbmissx
-------------------

.. c:function:: void H2K_handle_tlbmissx(void)

Description
~~~~~~~~~~~

Handles a TLB miss caused by an instruction fetch.

Functionality
~~~~~~~~~~~~~

The handler saves the interrupted context and, when ``COUNT_TLB_EVENTS`` is
enabled, atomically increments the current VM's execute-TLB-miss counter.
It examines the exception cause to select the address that missed:

* for a second-page instruction miss, it uses ``ELR + 16``;
* for an instruction-cache-invalidate miss, it uses ``BADVA``;
* otherwise, it uses ``ELR``.

It then transfers control to the common TLB-miss path.

H2K_handle_tlbmissrw
--------------------

.. c:function:: void H2K_handle_tlbmissrw(void)

Description
~~~~~~~~~~~

Handles a TLB miss caused by a data load or store.

Functionality
~~~~~~~~~~~~~

The handler saves the interrupted context and, when ``COUNT_TLB_EVENTS`` is
enabled, atomically increments the current VM's read/write-TLB-miss counter.
It uses ``BADVA`` as the address that missed, then calls
:c:func:`H2K_mem_tlb_fill()` through the common path.  The TLB-fill routine
resolves and inserts the translation, or handles the page fault when no
translation exists.  Finally, the interrupted context is restored.
