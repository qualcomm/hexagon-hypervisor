
:mod:`alloc` -- Memory allocator
================================

.. module:: alloc

Overview
--------

This allocator uses address-ordered first-fit with boundary tags to allocate
memory from a fixed-size region.  We assume that the allocator is not called
frequently, that the number of allocated blocks is "small", and that the caller
leaves the tag words untouched.  In this case we can use the boundary tags to
traverse the the space when searching for a free block without making
allocation unduly slow.  We reduce overhead since we have no external free
list, reduce code size somewhat, and marginally speed up the allocate/free
bookkeeping.

# .. c:type:: H2K_mem_alloc_block_t

#   Allocation return value

# 	.. c:member:: u32_t ptr

#     Pointer to 32-byte-aligned allocated region

# 	.. c:member:: u32_t size

# 	  Actual size of allocated region, in bytes


H2K_mem_alloc_get
-----------------

.. c:function:: H2K_mem_alloc_block_t H2K_mem_alloc_get(u32_t request)

	 :param request: Size of requested block, in bytes

Description
~~~~~~~~~~~

Return a pointer to a 32-byte-aligned region of at least the requested size, or
NULL if the request cannot be satisfied.



Functionality
~~~~~~~~~~~~~

The allocated size is a multiple of 32 bytes, less one word.  The first
suitable block (in address order) is used, and split if the remainder is at
least 32 bytes.  Since we allocate in units of 32 bytes, the only possible
remainders are >=32 or 0.


H2K_mem_alloc_free
------------------

.. c:function:: u32_t H2K_mem_alloc_free(u32_t *ptr)

	 :param ptr: Pointer to region to be freed

Description
~~~~~~~~~~~

Free the given region. Return pointer to tag of the freed region.

Functionality
~~~~~~~~~~~~~

Merge with any adjacent free regions.


H2K_mem_alloc_release
---------------------

.. c:function:: void H2K_mem_alloc_release(u32_t *ptr)

	 :param ptr: Pointer to region to be released

Description
~~~~~~~~~~~

Mark the given region as freeable.  Must be called with BKL held.  The region
can be accessed until BKL_UNLOCK().

Functionality
~~~~~~~~~~~~~

Set the released bit in the tag for the given block.  The next time that
H2K_mem_alloc_get() encounters the block, it will free it under the BKL---after
the caller of H2K_mem_alloc_release() has finished using the block and
unlocked.  This is used by H2K_thread_stop(), which may release a vmblock
before it enters H2K_switch(), which reads from the block before unlocking.


H2K_mem_alloc_init
------------------

.. c:function:: void H2K_mem_alloc_init()

Description
~~~~~~~~~~~

Initialize the allocation space.
