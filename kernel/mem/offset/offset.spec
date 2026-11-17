:mod:`offset` -- Offset translations
====================================

.. module:: offset

H2K_offset_translate
--------------------

.. c:function:: H2K_translation_t H2K_offset_translate(H2K_translation_t in, H2K_asid_entry_t info)

	:param in: Incoming translation values
	:param info: ASID entry containing the offset descriptor and VM identity
	:returns: The translated result, or a zero translation when the address is outside the permitted range

Description
~~~~~~~~~~~

Translates an address using the offset descriptor stored in the ASID entry.

Functionality
~~~~~~~~~~~~~

Adds the descriptor's page offset to the incoming physical page number and
limits the page size to the descriptor's size.  It applies the descriptor's
permissions and cache attributes, then follows the VM's parent guest mapping
when the result is not shared.

At the lowest translation level, TCM and VTCM addresses bypass the offset and
fence checks.  Other addresses must lie between the VM block's ``fence_lo``
and ``fence_hi`` limits; addresses outside that range produce a zero
translation.
