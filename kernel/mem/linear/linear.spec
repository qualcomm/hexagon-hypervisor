

:mod:`linear` -- Linear Memory Map Definition
=============================================

.. module:: linear

The linear memory map definition is an array of 64-bit values.

These values must be pinned into kernel memory.  The format is::

	INSERT FORMAT HERE

The values must terminate with a value zero.

H2K_mem_translate_linear
------------------------

.. cfunction:: H2K_linear_fmt_t H2K_mem_translate_linear(u32_t badva, H2K_thread_context *me)
	:param badva: Virtual Address to translate
	:param me: Context of the current thread

Description
~~~~~~~~~~~

The :cfunc:`H2K_mem_translate_linear()` function searches a linear list of
translations for a match for the virtual address "badva".  If such a
translation is found, it is converted to the native TLB format and returned.
Otherwise, we return all zeros.

Functionality
~~~~~~~~~~~~~

The list is located at me->gptb.

For each entry in the list, we first calculate the size of the translation.
We then mask both the "badva" field, as well as the the VA field of the entry.
If these two are equal, we have a match.  If there is no match, we continue
to the next entry in the list.  If we find a zero entry, we have unsuccessfully
searched the entire list, and so return zero.



