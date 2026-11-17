
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

The allocator acquires the BKL before ``alloc_heap_lock`` while searching for
and reclaiming released blocks.  This order must be preserved because
thread-stop paths may hold the BKL while freeing a VM block.  The BKL is
released after the search/reclamation phase; ``alloc_heap_lock`` remains held
while the allocation metadata is updated.  If no suitable block exists, both
locks are released before returning a zero-valued result.


H2K_mem_alloc_free
------------------

.. c:function:: H2K_mem_alloc_tag_t *H2K_mem_alloc_free(void *ptr)

	 :param ptr: Pointer to the allocated region to be freed

Description
~~~~~~~~~~~

Free the given region and return a pointer to its boundary tag.

Functionality
~~~~~~~~~~~~~

Acquire ``alloc_heap_lock``, mark the block free, merge it with adjacent free
regions, update the boundary tags, and release ``alloc_heap_lock``.  The
caller must pass a pointer previously returned by
:c:func:`H2K_mem_alloc_get()`.


H2K_mem_alloc_init
------------------

.. c:function:: void H2K_mem_alloc_init(H2K_mem_alloc_tag_t addr[], u32_t size)

	 :param addr: Start of the allocator's backing region
	 :param size: Size of the backing region in allocator tag words

Description
~~~~~~~~~~~

Initialize the allocation space and its heap lock.  The backing region is
initialized as one free block with a sentinel tag at its end.  This function
must run before any allocation or free operation.
