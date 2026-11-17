:mod:`pagewalk` -- Page-table translation
============================================

.. module:: pagewalk

H2K_pagewalk_translate
----------------------

.. c:function:: H2K_translation_t H2K_pagewalk_translate(H2K_translation_t in, H2K_asid_entry_t info)

	:param in: Incoming translation values, including the virtual page number
	:param info: ASID entry identifying the page tables and guest
	:returns: The refined translation, or a zero translation when the mapping is invalid

Description
~~~~~~~~~~~

Translates an address by walking the guest's level-1 and, when required,
level-2 page tables.

Functionality
~~~~~~~~~~~~~

The level-1 page-table base is taken from ``info.ptb``.  When the VM has a
guest mapping, the page-table address is translated through that mapping and
must have read permission.  The level-1 entry is read from physical memory.

For a level-1 entry that points to a level-2 table, the relevant level-2 entry
is located using the incoming virtual page number.  The level-2 table address
is translated through the VM mapping when one exists and must also be readable.
The level-2 page-size field is replaced with the level-1 page size before the
entry is used.

An invalid level-1 page size or an inaccessible page-table mapping produces a
zero PTE.  Otherwise, the selected PTE refines the incoming translation's
page number, page size, permissions, cache attribute, and shared attribute.
If the result is not shared and the VM has a guest mapping, the refined
translation is passed through that mapping before it is returned.
