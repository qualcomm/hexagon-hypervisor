
:mod:`readylist` -- managing unblocked, non-executing threads
==============================================================

.. module:: readylist

H2K_ready and H2K_ready_valids
------------------------------

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

We find the highest priority ready thread by using the CL0 instruction to find
the highest priority that has a ready thread.  We then can remove the thread in
the corresponding list.


H2K_ready_init
--------------

.. cfunction:: void H2K_ready_init()

Description
~~~~~~~~~~~

Initializes the H2K_ready structures.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

Set all elements of H2K_ready to NULL, and set H2K_ready_valids to zero.


H2K_ready_best_prio
-------------------

.. cfunction:: static inline u32_t H2K_ready_best_prio()

Description
~~~~~~~~~~~

Returns the priority corresponding to the ready thread with the best priority.

Input
~~~~~

Output
~~~~~~

Returns the priority corresponding to the ready thread with the best priority.

Functionality
~~~~~~~~~~~~~

Count Trailing Zeros of H2K_ready_valids.



H2K_ready_append
----------------

.. cfunction:: static inline void H2K_ready_append(H2K_thread_context *thread)

Description
~~~~~~~~~~~

Appends the thread to the ready list.

Input
~~~~~

Argument 0: the thread to add

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

We get the priority from the thread context.  Next, we call H2K_ring_append
on the H2K_ready ring at the thread's priority.  Finally, we set the bit of
H2K_ready_valids at the thread's priority.



H2K_ready_insert
----------------

.. cfunction:: static inline void H2K_ready_insert(H2K_thread_context *thread)

Description
~~~~~~~~~~~

Inserts the thread to the ready list.

Input
~~~~~

Argument 0: the thread to add

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

We get the priority from the thread context.  Next, we call H2K_ring_insert
on the H2K_ready ring at the thread's priority.  Finally, we set the bit of
H2K_ready_valids at the thread's priority.



H2K_ready_remove
----------------

.. cfunction:: static inline void H2K_ready_remove(H2K_thread_context *thread)

Description
~~~~~~~~~~~

Removes the thread from the ready list.

Input
~~~~~

Argument 0: the thread to remove

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

We get the priority from the thread context.  Next, we call H2K_ring_remove
on the H2K_ready ring at the thread's priority.  Finally, if there are no
more elements in the ring, we clear the bit of H2K_ready_valids at the
thread's priority.


H2K_ready_getbest
-----------------

.. cfunction:: static inline H2K_thread_context *H2K_ready_getbest()

Description
~~~~~~~~~~~

Removes the best priority thread from the ready list.

Input
~~~~~

Output
~~~~~~

Returns the best priority ready thread, which has been removed from the ready list.
Returns NULL if no threads are ready.

Functionality
~~~~~~~~~~~~~

If there are no ready threads, we return NULL.

We call H2K_ready_best_prio() to obtain the priority of the best priority ready thread.

We then get the thread pointed to by the H2K_ready pointer at the correct priority.

This thread is removed from the ready list by calling H2K_ready_remove, and returned.

