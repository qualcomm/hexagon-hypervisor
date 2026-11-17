:mod:`linear` -- Linear memory-map translation
================================================

.. module:: linear

The linear memory map is a list of 64-bit translation descriptors.  A zero
entry terminates the list; a chain entry redirects the lookup to another list.

H2K_linear_translate
--------------------

.. c:function:: H2K_translation_t H2K_linear_translate(H2K_translation_t in, H2K_asid_entry_t info)

	:param in: Incoming translation values, including the virtual page number
	:param info: ASID entry identifying the linear map and its guest
	:returns: The refined translation, or a zero translation when no entry matches

Description
~~~~~~~~~~~

Translates an address through the linear map identified by an ASID entry.

Functionality
~~~~~~~~~~~~~

The function reads linear-map entries from the physical address represented by
``info.ptb``.  When a guest page mapping is present, each list page is
translated through the guest mapping before it is read; an entry requiring
read access cannot be used if the mapping lacks read permission.

A zero entry ends the search.  A chain entry redirects the search to another
linear-map list.  For a normal entry, the virtual-page mask is compared with
the incoming virtual page number.  On a match, the entry refines the physical
page number, page size, permissions, cache attributes, weak-cache flag, and
shared flag.  If the result is not shared and the guest has a parent mapping,
the refined translation is passed to the parent translation.  If no matching
entry is found, a zero translation is returned.
