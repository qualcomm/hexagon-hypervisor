
:mod:`readylist` -- managing unblocked, non-executing threads
=============================================================

.. module:: readylist

H2K_kg.ready and H2K_kg.ready_valids
------------------------------------

Description
~~~~~~~~~~~

The ready list contains the threads that are ready for execution.

The requirements for the ready list are:

* O(1) insertions of any priority
* O(1) removals of the highest priority
* Very fast detection of the highest priority ready thread

We achieve this by using an array of linked lists of threads ready at a given
priority, and a bitmask that has a bit set for each non-empty linked list.

We insert a thread into the ready list by adding the thread to the list at the
priority corresponding to the thread, and setting the corresponding bit.

We find the highest priority ready thread by using the CT0 instruction to find
the highest priority that has a ready thread.  We then can remove the thread in
the corresponding list.

H2K_ready_init
--------------

.. cfunction:: void H2K_ready_init()

Description
~~~~~~~~~~~

Initializes the H2K_kg.ready structures.

Functionality
~~~~~~~~~~~~~

Set all elements of H2K_kg.ready to NULL, and set H2K_kg.ready_valids to zero.


H2K_ready_best_prio
-------------------

.. cfunction:: static inline u32_t H2K_ready_best_prio()

	:returns: the priority corresponding to the ready thread with the best priority.
		Returns a value of MAX_PRIOS or higher if no threads are ready.

Description
~~~~~~~~~~~

Functionality
~~~~~~~~~~~~~

Count Trailing Zeros of H2K_kg.ready_valids.



H2K_ready_any_valid
-------------------

.. cfunction:: static inline u32_t H2K_ready_any_valid()

	:returns: whether any threads are ready.

Description
~~~~~~~~~~~

Functionality
~~~~~~~~~~~~~

Tests whether :cfunc:`H2K_ready_best_prio()` returns MAX_PRIOS or higher.



H2K_ready_prio_valid
--------------------

.. cfunction:: static inline u32_t H2K_ready_prio_valid(u32_t prio)

	:param prio: the priority to check
	:returns: whether a thread at the given priority is ready.

Description
~~~~~~~~~~~

Checks whether a thread at a given priority is ready.

Functionality
~~~~~~~~~~~~~

Check the bit in ready_valids corresponding to the priority.



H2K_ready_set_prio
------------------

.. cfunction:: static inline void H2K_ready_set_prio(u32_t prio)

	:param prio: the priority to set

Description
~~~~~~~~~~~

Sets a given priority as valid.

Functionality
~~~~~~~~~~~~~

Set the bit in ready_valids corresponding to the priority.



H2K_ready_clear_prio
--------------------

.. cfunction:: static inline void H2K_ready_clear_prio(u32_t prio)

	:param prio: the priority to clear

Description
~~~~~~~~~~~

Sets a given priority as invalid.

Functionality
~~~~~~~~~~~~~

Clear the bit in ready_valids corresponding to the priority.



H2K_ready_append
----------------

.. cfunction:: static inline void H2K_ready_append(H2K_thread_context *thread)

	:param thread: the thread to add

Description
~~~~~~~~~~~

Appends the thread to the ready list.

Functionality
~~~~~~~~~~~~~

We get the priority from the thread context.  Next, we call :cfunc:`H2K_ring_append()`
on the H2K_kg.ready ring at the thread's priority.  Finally, we call
:cfunc:`H2K_ready_set_prio()` with the thread's priority.



H2K_ready_insert
----------------

.. cfunction:: static inline void H2K_ready_insert(H2K_thread_context *thread)

	:param thread: the thread to add

Description
~~~~~~~~~~~

Inserts the thread to the ready list.

Functionality
~~~~~~~~~~~~~

We get the priority from the thread context.  Next, we call :cfunc:`H2K_ring_insert()`
on the H2K_kg.ready ring at the thread's priority.  Finally, we call
:cfunc:`H2K_ready_set_prio()` with the thread's priority.



H2K_ready_remove
----------------

.. cfunction:: static inline void H2K_ready_remove(H2K_thread_context *thread)

	:param thread: the thread to remove

Description
~~~~~~~~~~~

Removes the thread from the ready list.

Functionality
~~~~~~~~~~~~~

We get the priority from the thread context.  Next, we call :cfunc:`H2K_ring_remove()`
on the H2K_kg.ready ring at the thread's priority.  Finally, if there are no
more elements in the ring, we call :cfunc:`H2K_ready_clear_prio()` with the
thread's priority.


H2K_ready_getbest
-----------------

.. cfunction:: static inline H2K_thread_context *H2K_ready_getbest()

	:returns: the best priority ready thread, which has been removed from
		the ready list.  Or, if no threads are ready, returns NULL.

Description
~~~~~~~~~~~

Removes the best priority thread from the ready list.

Functionality
~~~~~~~~~~~~~

If there are no ready threads (!:cfunc:`H2K_ready_any_valid()`), we return NULL.

We call :cfunc:`H2K_ready_best_prio()` to obtain the priority of the best priority ready thread.

We then get the thread pointed to by the H2K_kg.ready pointer at the correct priority.

This thread is removed from the ready list by calling :cfunc:`H2K_ready_remove()`, and returned.






Testing
-------

Samples
~~~~~~~

* Input: H2K_kg.ready_valids
* Input: H2K_kg.ready array
* Thread to append/insert/remove
* Output: H2K_ready_best_prio: Priority of best ready thread
* Output: H2K_ready_getbest: Best ready thread, removed from readylist, or NULL
* Output: H2K_ready_array / H2K_kg.ready_valids for append/insert/remove/getbest


Important Cases
~~~~~~~~~~~~~~~

* H2K_ready_getbest when ready list is empty
* H2K_ready_getbest when ready list has one thread
* H2K_ready_getbest when ready list has multiple threads at same priority level
* H2K_ready_getbest when ready list has multiple threads at multiple priority levels
* H2K_ready_best_prio when ready list has valid threads
* H2K_ready_best_prio when ready list is empty
* H2K_ready_append to empty list
* H2K_ready_append to non-empty list, check for thread at end of list
* H2K_ready_insert to empty list
* H2K_ready_insert to non-empty list, check for thread at start of list
* H2K_ready_remove to list with only the specified thread, check for H2K_kg.ready_valids bit clear
* H2K_ready_remove to list with more than just the specified thread, check for H2K_kg.ready_valids bit remaining set
* Check H2K_ready_init clears out randomized values

Harness
~~~~~~~

The readylist module is reasonably self-contained, so the test harness will only
use the header file and object file.  


