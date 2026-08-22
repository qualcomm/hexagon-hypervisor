
ASM_REF_CODE(Data with a label, easier to represent in asm)

:mod:`bootparams` -- boot parameter block
=========================================

.. module:: bootparams

H2K_boot_params
---------------

Boot parameters captured at reset.

Description
~~~~~~~~~~~

Platform boot firmware may hand the kernel parameters in r1:0 at reset; a
bootloader passes the physical address of a generated device tree blob there.
The entry code in the ``boot`` module stores those registers into this block
before anything clobbers them, and :c:func:`H2K_init_setup` copies the words
into the kernel globals as ``boot_r0``/``boot_r1`` once
:c:func:`H2K_kg_init` has zeroed them.  Guests read them back through the
``INFO_BOOT_R00`` and ``INFO_BOOT_R01`` info trap operations.

The block is BOOT_PARAMS_WORDS words long; the words after the first two are
reserved for future parameters.  It is doubleword aligned because the capture
path stores r1:0 with a single ``memd``.

This lives in its own module, rather than in the entry page alongside the
angel mailboxes, so that referring to it does not pull the boot module into a
link.  That module defines ``start``, which collides with the C runtime
startup object in STANDALONE tests.  For the same reason it is kept out of
the ``globals`` module, whose symbols those tests stub locally.  It is not a
member of :c:type:`H2K_kg_t` because the capture happens before
:c:func:`H2K_kg_init` zeroes the globals.
