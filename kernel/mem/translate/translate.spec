:mod:`translate` -- Translate an address
========================================

.. module:: translate

H2K_translate
-------------

.. cfunction:: H2K_translation_t H2K_translate(u32_t addr, H2K_vmblock_t *vmblock)

	:param addr: Address to translate
	:param vmblock: vmblock pointer

Description
~~~~~~~~~~~

Translates the given address using translation base and type from vmblock.

Functionality
~~~~~~~~~~~~~

Call the appropriate lookup function according to translation type.  If successful, call the corresponding translation function and return its result.  Else return 0 (invalid translation).

