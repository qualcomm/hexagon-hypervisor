:mod:`tlbfmt` -- Hardware TLB entry format
=============================================

.. module:: tlbfmt

H2K_mem_tlbfmt_t
----------------

.. c:type:: H2K_mem_tlbfmt_t

	Native 64-bit hardware TLB entry format.

	.. c:member:: u64_t raw

		Raw access to the complete TLB entry.

	.. c:member:: u32_t low

		Raw access to the low 32 bits of the entry.

	.. c:member:: u32_t ppd

		Physical page description, including the physical page number and page-size
		encoding.

	.. c:member:: u32_t cccc

		Cacheability attributes.

	.. c:member:: u32_t hsv39

		Indicates that the entry uses the V39/DMA address-space format.  Present in
		the current format on architectures that support this field.

	.. c:member:: u32_t xwru

		Execute, write, read, and user permission bits.

	.. c:member:: u32_t high

		Raw access to the high 32 bits of the entry.

	.. c:member:: u32_t vpn

		Virtual page number.

	.. c:member:: u32_t asid

		Address-space identifier.

	.. c:member:: u32_t abits

		Architecture-specific address bits for architectures before V73.

	.. c:member:: u32_t pa3637

		Physical-address bits 36 and 37 on V73 and later architectures.

	.. c:member:: u32_t pa35

		Physical-address bit 35.

	.. c:member:: u32_t global

		Global-entry bit.  A global entry does not use the ASID for matching.

	.. c:member:: u32_t valid

		Indicates that the TLB entry is valid.

H2K_mem_tlbfmt_get_perms
------------------------

.. c:function:: static inline u32_t H2K_mem_tlbfmt_get_perms(H2K_mem_tlbfmt_t entry)

	:param entry: TLB entry
	:returns: The entry's execute, write, read, and user permission bits

Description
~~~~~~~~~~~

Returns the permission field from a TLB entry.


H2K_mem_tlbfmt_get_size
-----------------------

.. c:function:: static inline u32_t H2K_mem_tlbfmt_get_size(H2K_mem_tlbfmt_t entry)

	:param entry: TLB entry
	:returns: The encoded page-size value

Description
~~~~~~~~~~~

Returns the page-size encoding by counting the trailing zero bits in the low
word of the entry.


H2K_mem_tlbfmt_get_basepa
-------------------------

.. c:function:: static inline pa_t H2K_mem_tlbfmt_get_basepa(H2K_mem_tlbfmt_t entry)

	:param entry: TLB entry
	:returns: The base physical address represented by the entry

Description
~~~~~~~~~~~

Reconstructs the base physical address from the physical page description and
the architecture-specific high physical-address bits.  The result is aligned
according to the encoded page size.


H2K_mem_tlbfmt_from_trans
-------------------------

.. c:function:: static inline H2K_mem_tlbfmt_t H2K_mem_tlbfmt_from_trans(H2K_translation_t trans, u32_t va, u32_t asid, u32_t tlb_size_dma)

	:param trans: Translation to encode
	:param va: Virtual address
	:param asid: Address-space identifier
	:param tlb_size_dma: DMA TLB size on V73 and later architectures
	:returns: Encoded TLB entry, or a zero entry when the translation is unusable

Description
~~~~~~~~~~~

Converts a software translation into the native hardware TLB-entry format.
On architectures before V73, the function has no ``tlb_size_dma`` parameter.

Functionality
~~~~~~~~~~~~~

Returns a zero entry when the translation has no read, write, or execute
permission, or when it requests the V39/DMA address space but no DMA TLB is
available.  Otherwise, it copies the virtual page number, ASID, permissions,
cache attributes, and address-space information into the hardware format,
marks the entry valid and non-global, and encodes the physical page number and
page size in ``ppd``.
