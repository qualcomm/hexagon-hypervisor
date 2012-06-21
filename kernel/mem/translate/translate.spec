:mod:`translate` -- Translate an address
========================================

.. module:: translate

H2K_translate
-------------

.. cfunction:: s32_t H2K_translate(u32_t addr, u32_t pmap, translation_type type, u32_t *result)

	:param addr: Address to translate
	:param pmap: Translation base
	:param type: Translation type
	:param result: pointer to result

Description
~~~~~~~~~~~

Translates the given address using given translation base and type.

Functionality
~~~~~~~~~~~~~

Call the appropriate lookup function according to translation type.  If successful, store result and return 0, else return -1.
