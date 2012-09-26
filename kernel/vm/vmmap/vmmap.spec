:mod:`vmmap` -- Address Translation Map Support
===============================================

.. module:: vmmap


H2K_vmtrap_clrmap
-----------------

.. cfunction:: void H2K_vmtrap_clrmap(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

This function handles the "vmclrmap" virtual instruction, to 
clear caches of address translations.

Functionality
~~~~~~~~~~~~~

This function must clear out any cached translations for the specified address
range, in both the hardware TLB, as well as any other structures that cache
address translations (such as the STLB).

We currently have any clrmap call invalidate all translations in the current
ASID.



H2K_vmtrap_newmap
-----------------

.. cfunction:: void H2K_vmtrap_newmap(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

This function handles the "vmnewmap" virtual instruction, to 
register a new set of address translations.

Functionality
~~~~~~~~~~~~~

This function checks to make sure that the translation type is valid,
and then registers the information with :cfunc:`H2K_asid_table_inc()`.
If successful, we decrement the reference count on the current ASID
with :cfunc:`H2K_asid_table_dec()`, and set the new ASID value.
If unsuccessful for any reason, we return -1 in r0.



