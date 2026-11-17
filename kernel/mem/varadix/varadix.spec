:mod:`varadix` -- Variable-radix address translation
====================================================

.. module:: varadix

A varadix map walks a variable number of VPN bits at each level.  The entry's
trailing-zero encoding determines the number of bits consumed by that level;
it is not a fixed 10-bit/10-bit page-table split.  ``VARADIX_PN_BITS`` is 22,
and the final page offset is handled by the translation size.

H2K_varadix_translate
---------------------

.. c:function:: H2K_translation_t H2K_varadix_translate(H2K_translation_t in, H2K_asid_entry_t info)

	:param in: Incoming translation, including the virtual page number
	:param info: ASID entry containing the root varadix entry and VM identity
	:returns: Refined translation, or a zero translation when the address is invalid

Description
~~~~~~~~~~~

Translates an address through a variable-radix translation tree.

Functionality
~~~~~~~~~~~~~

The number of valid virtual-page bits comes from ``info.fields.extra``.  The
root entry is examined first.  If it does not cover all requested bits, the
walker uses the entry's encoded radix size to select an index from the VPN,
forms the next table address, and continues until a leaf entry is reached.

When a VM has a parent ``guestmap``, each table address is translated through
that mapping and must be readable.  A leaf entry supplies the physical page,
permissions, cache attributes, and translation size.  The result is merged
with the incoming translation and, when it is not shared, is passed through
the parent guest mapping.

An address outside the configured virtual-page range, an invalid radix size,
or an unreadable table produces a zero translation.

H2K_varadix_entry_size
----------------------

Returns the size encoded in a varadix entry.  The size is obtained from the
number of trailing zero bits in the entry and determines how the entry is used
in the walk.


H2K_varadix_walk
----------------

Selects entries from the varadix tables using the relevant VPN bits.  It
translates each table address through the VM's parent mapping when necessary,
then follows child tables until it reaches a leaf or detects an invalid walk.
The loop terminates on either condition; it is not an unbounded loop.


H2K_varadix_mktrans
-------------------

Converts a leaf entry into an ``H2K_translation_t``.  The result contains the
leaf's physical page, permissions, cache attributes, and page-size value.


H2K_varadix_update
------------------

Combines a leaf translation with the incoming translation.  Permissions are
intersected and the smaller page size is retained.  If the leaf is invalid,
the invalid translation is returned.
