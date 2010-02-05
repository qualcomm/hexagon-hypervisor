
:mod:`tlbmiss` -- TLB Miss Handling
===================================

.. module:: tlbmiss


H2K_handle_tlbmissx
-------------------

.. cfunction:: void H2K_handle_tlbmissx(void)


Description
~~~~~~~~~~~

This code handles a TLB Miss due to an instruction fetch. 

Functionality
~~~~~~~~~~~~~

* Save off registers
* Look at cause
* If icinva miss, bad virtual address is BADVA
* If second-page instruction miss, bad virtual address is ELR+16
* If normal instruction miss, bad virtual address is ELR
* Once bad virtual address has been calculated, jump to appropriate
  location in :cfunc:`H2K_handle_tlbmissrw()`

H2K_handle_tlbmissrw
--------------------

.. cfunction:: void H2K_handle_tlbmissrw(void)


Description
~~~~~~~~~~~

This code handles a TLB miss due to a load or store.

Functionality
~~~~~~~~~~~~~

First, we do the tlbmissrw-specific code:
* Save off registers
* Bad virtual address is BADVA

Next, we do code that is common to both RW and X misses:
* Save off additional registers
* Set up kernel stack and :cvar:`H2K_gp`
* Call TLB Fill routine: :cfunc:`H2K_mem_tlb_fill()`
* Restore registers
* Return, unlocking TLB

