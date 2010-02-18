
:mod:`tlbfmt` -- Hardware TLB Entry Format Services
===================================================

.. module:: tlbfill

H2K_mem_tlbfmt_t
----------------

.. ctype:: H2K_mem_tlbfmt_t


H2K_mem_tlbfmt_from_linear
--------------------------

.. cfunction:: H2K_mem_tlbfmt_t H2K_mem_tlbfmt_from_linear(H2K_linear_t linear, u32_t asid)

Description
~~~~~~~~~~~

This function converts a linear translation table entry and ASID to the native
Hardware TLB entry format.

H2K_mem_tlbfmt_from_table
-------------------------

.. cfunction:: H2K_mem_tlbfmt_t H2K_mem_tlbfmt_from_table(u32_t va, u32_t asid, u32_t pte)

Description
~~~~~~~~~~~

This function converts a page table entry, VA, and ASID to the native Hardware
TLB entry format.


