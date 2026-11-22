
:mod:`tcm` -- Tightly-Coupled Memory boot support
==================================================

.. module:: tcm

H2K_tcm_copy
------------

.. c:function:: void H2K_tcm_copy(u32_t last_tlb_index)

	:param last_tlb_index: Last regular TLB index in use before this call

Description
~~~~~~~~~~~

Copies the kernel's boot pages into Tightly-Coupled Memory (TCM), when
``H2K_USE_TCM`` is enabled and TCM is large enough to hold the kernel.

Functionality
~~~~~~~~~~~~~

Computes the TCM base and size, and checks whether TCM is large enough to
hold the kernel's boot pages.  If so, it temporarily maps the TCM region and,
for each kernel boot page, copies the page's contents to TCM and replaces the
TLB entry mapping that page so it now points at the freshly copied page in
TCM instead of its original location.  The temporary TCM mapping is removed
once every page has been copied.

After the copy completes, the function records that TCM is in use and
recomputes the kernel's physical offset relative to the new TCM location.


H2K_tcm_crash_copy
------------------

.. c:function:: void H2K_tcm_crash_copy()

Description
~~~~~~~~~~~

Copies TCM contents into the crash-dump area for diagnostics.  Available only
when ``CRASH_DEBUG`` is enabled.

Functionality
~~~~~~~~~~~~~

Clears the software-TLB pointer, in case it referenced the TCM crash area,
then reads TCM word by word using :c:func:`H2K_mem_physread_word()` and writes
the words into the crash TCM destination area, up to a fixed maximum number of
bytes.
