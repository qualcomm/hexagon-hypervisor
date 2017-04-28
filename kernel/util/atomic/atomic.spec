

:mod:`atomic` -- Atomic modifications of memory
===============================================

.. module:: atomic


H2K_atomic_setbit
-----------------

.. c:function:: static inline H2K_atomic_setbit(u32_t *word, u32_t bit)

Description
~~~~~~~~~~~

This sets the bit corresponding to `bit` in the word pointed to by `word`.

Functionality
~~~~~~~~~~~~~

We use the hardware atomic memory primitives to set the bit in a manner that
works even if other threads are attempting to modify the same word in memory.


H2K_atomic_clrbit
-----------------

.. c:function:: static inline H2K_atomic_clrbit(u32_t *word, u32_t bit)

Description
~~~~~~~~~~~

This clears the bit corresponding to `bit` in the word pointed to by `word`.

Functionality
~~~~~~~~~~~~~

We use the hardware atomic memory primitives to clear the bit in a manner that
works even if other threads are attempting to modify the same word in memory.



