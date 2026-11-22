:mod:`translate` -- Translate an address
========================================

.. module:: translate

H2K_translate
-------------

.. c:function:: H2K_translation_t H2K_translate(H2K_translation_t in, H2K_asid_entry_t info)

	:param in: Incoming translation
	:param info: ASID entry selecting the translation method
	:returns: Translated result, or zero if invalid

Description
~~~~~~~~~~~

Dispatches the translation through the method selected by
``info.fields.type``.

Functionality
~~~~~~~~~~~~~

Calls the corresponding linear, page-table, offset, or varadix translator
with ``in`` and ``info`` and returns its result.
