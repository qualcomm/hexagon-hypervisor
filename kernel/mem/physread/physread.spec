
:mod:`physread` -- Read Memory from Physical Address
====================================================

.. module:: physread




H2K_mem_physread_word
---------------------

.. c:function:: u32_t H2K_mem_physread_word(u64_t pa)

	:param pa: Physical address 

Description
~~~~~~~~~~~

The :c:func:`H2K_mem_physread_word()` function reads a word at a chosen physical
address.  It is assumed to be L1 and L2 cacheable.  

Functionality
~~~~~~~~~~~~~

For V4, we use the memw_phys instruction.  For V3, we set up a temporary mapping
in monitor memory space (upper 16MB) and read from there.

Testing
-------

Tested cases
~~~~~~~~~~~~

* Perform read

H2K_mem_physread_dword
----------------------

.. c:function:: u64_t H2K_mem_physread_dword(u64_t pa)

	:param pa: Physical address 

Description
~~~~~~~~~~~

The :c:func:`H2K_mem_physread_dword()` function reads a doubleword at a chosen
physical address.  It is assumed to be L1 and L2 cacheable.  

Functionality
~~~~~~~~~~~~~

For V4, we use two memw_phys instructions.  For V3, we set up a temporary mapping
in monitor memory space (upper 16MB) and read from there.


Testing
-------

Tested cases
~~~~~~~~~~~~

* Perform read
