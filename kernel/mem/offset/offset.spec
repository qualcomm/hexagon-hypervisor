:mod:`offset` -- offset translations
====================================

.. module:: offset

H2K_offset_translate
--------------------

.. cfunction:: H2K_translation_t H2K_offset_translate(H2K_translation_t in, H2K_asid_entry_t *info)

	:param in: incoming translation values
	:param info: pointer to info for this translation

Description
~~~~~~~~~~~

Translations the input translation and returns an output translation.  If translation is invalid, return 0.

Functionality
~~~~~~~~~~~~~

Offset translation is fairly easy.  Adjust values in the translation, and either recurse to the parent 
translation format or return.

