:mod:`atomic` -- Atomic memory operations
============================================

.. module:: atomic

All operations use Hexagon load-linked/store-conditional primitives and retry
when another hardware thread changes the word during the operation.  The
returned value is operation-specific and is described below.


Bit operations
--------------

.. c:function:: static inline u32_t H2K_atomic_setbit(u32_t *word, u32_t bit)

	:param word: Word to modify
	:param bit: Bit position to set
	:returns: 1 if the bit was clear before the operation, otherwise 0

Atomically sets ``bit`` in ``*word``.


.. c:function:: static inline u32_t H2K_atomic_clrbit(u32_t *word, u32_t bit)

	:param word: Word to modify
	:param bit: Bit position to clear
	:returns: 1 if the bit was set before the operation, otherwise 0

Atomically clears ``bit`` in ``*word``.


Word operations
---------------

.. c:function:: static inline u32_t H2K_atomic_swap(u32_t *word, u32_t val)

	:param word: Word to replace
	:param val: New value
	:returns: Value of ``*word`` before replacement

Atomically replaces ``*word`` with ``val``.


.. c:function:: static inline u32_t H2K_atomic_compare_swap(u32_t *word, u32_t expect, u32_t val)

	:param word: Word to inspect
	:param expect: Expected current value
	:param val: Replacement value
	:returns: Value read from ``*word``

Atomically compares ``*word`` with ``expect``.  If they are equal, stores
``val``; otherwise leaves the word unchanged.  The value read before the
conditional store is returned in either case.


.. c:function:: static inline u32_t H2K_atomic_insert(u32_t *word, u32_t val, u32_t width, u32_t offset)

	:param word: Word to modify
	:param val: Value to insert
	:param width: Width of the field
	:param offset: Starting bit position
	:returns: New value of the word

Atomically inserts the selected field from ``val`` into ``*word`` and returns
the resulting word.


.. c:function:: static inline u32_t H2K_atomic_add(u32_t *word, u32_t val)

	:param word: Word to modify
	:param val: Value to add
	:returns: New word value

Atomically adds ``val`` to ``*word``.


.. c:function:: static inline u64_t H2K_atomic_add64(u64_t *word, u64_t val)

	:param word: 64-bit word to modify
	:param val: Value to add
	:returns: New 64-bit word value

Atomically adds ``val`` to a 64-bit word.


.. c:function:: static inline u32_t H2K_atomic_or(u32_t *word, u32_t val)

	:param word: Word to modify
	:param val: Bits to set
	:returns: New word value

Atomically computes ``*word |= val``.


Masked operations
-----------------

The masked operations modify or compare only ``*word & mask``.  ``val`` and
``expect`` must already be positioned within ``mask``; bits outside the mask
must be zero in those arguments.  The returned value is the value of the
selected field before the operation.

.. c:function:: static inline u32_t H2K_atomic_add_mask(u32_t *word, u32_t val, u32_t mask)

	:returns: Previous value of ``*word & mask``

Atomically adds ``val`` to the masked field, retaining all bits outside
``mask``.


.. c:function:: static inline u32_t H2K_atomic_max_mask(u32_t *word, u32_t val, u32_t mask)

	:returns: Previous value of ``*word & mask``

Atomically replaces the masked field with the maximum of its current value and
``val``.


.. c:function:: static inline u32_t H2K_atomic_compare_swap_mask(u32_t *word, u32_t expect, u32_t val, u32_t mask)

	:returns: Previous value of ``*word & mask``

Atomically compares the masked field with ``expect``.  If equal, replaces the
masked field with ``val``; bits outside ``mask`` are preserved.
