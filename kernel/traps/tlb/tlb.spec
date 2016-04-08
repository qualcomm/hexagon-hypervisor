
:mod:`tlb` -- direct manipulation of the TLB
============================================

.. module:: tlb


H2K_tlb_tlballoc
----------------

.. cfunction:: s64_t H2K_tlb_tlballoc(u32_t unused0, u32_t unused1, u64_t entry, H2K_thread_context *me)

	:param entry: TLB entry
	:param me: Pointer to the thread context of the currently-executing thread
	:returns: Current total time the thread has been scheduled, in processor cycles.

Description
~~~~~~~~~~~

Allocates place for a given pinned entry in the TLB, and inserts the entry there

Functionality
~~~~~~~~~~~~~

Returns the index of the entry.  Negative value on failure.


H2K_tlb_tlbfree
---------------

.. cfunction:: s64_t H2K_tlb_tlbfree(u32_t unused0, u32_t index, u64_t unused32, H2K_thread_context *me)


	:param index: Index of TLB entry to remove
	:param me: Pointer to the thread context of the currently-executing thread
	:returns: zero

Description
~~~~~~~~~~~

Frees the TLB entry

Functionality
~~~~~~~~~~~~~

Frees the TLB entry.  While the TLB entry at max_replacable_idx is free, increment max_replaceable_idx

H2K_tlb_tlbquery
----------------

.. cfunction:: s64_t H2K_tlb_tlbquery(u32_t unused0, u32_t va, u64_t unused32, H2K_thread_context *me)

	:param va: VA to search
	:param me: Pointer to the thread context of the currently-executing thread
	:returns: zero

Description
~~~~~~~~~~~

Searches for a TLB entry.

Functionality
~~~~~~~~~~~~~

Searches for a TLB entry. Returns 

H2K_tlb_tlbread
---------------

.. cfunction:: s64_t H2K_tlb_tlbread(u32_t unused0, u32_t idx, u64_t unused32, H2K_thread_context *me)

	:param idx: index to read
	:param me: Pointer to the thread context of the currently-executing thread
	:returns: zero

Description
~~~~~~~~~~~

Reads a TLB entry

Functionality
~~~~~~~~~~~~~

Reads a TLB entry.

H2K_tlb_tlbwrite
----------------

.. cfunction:: s64_t H2K_tlb_tlbwrite(u32_t unused0, u32_t idx, u64_t entry, H2K_thread_context *me)

	:param idx: index to read
	:param entry: TLB entry to write
	:param me: Pointer to the thread context of the currently-executing thread
	:returns: zero

Description
~~~~~~~~~~~

writes a TLB entry

Functionality
~~~~~~~~~~~~~

Writes a TLB entry. Returns zero


H2K_tlb_tlbop
-------------

.. cfunction:: s64_t H2K_tlb_tlbop(u32_t op, u32_t idx, u64_t entry, H2K_thread_context *me)

	:param op: what operation to do
	:param arg32: 32-bit argument
	:param arg64: 64-bit argument
	:param me: Pointer to the thread context of the currently-executing thread
	:returns: value of called function

Description
~~~~~~~~~~~

Calls the TLB function

Functionality
~~~~~~~~~~~~~

Calls the TLB function





Testing
-------

Important Cases
~~~~~~~~~~~~~~~

* TLB area full
* TLB area empty

Harness
~~~~~~~

Ensure we can manipulate TLB.

We will be in standalone so we should be able to look at TLB from TH to make sure the values are right.

Whitebox look at replaceable_idx to make sure it is moving correctly.

