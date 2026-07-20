
:mod:`readylist` -- managing unblocked, non-executing threads
=============================================================

.. module:: readylist

Overview
--------

The readylist API is split into two groups:

* **Base functions** (:c:func:`H2K_ready_append`, :c:func:`H2K_ready_insert`,
  :c:func:`H2K_ready_remove`) — manipulate the ready list without touching the
  hardware BESTWAIT comparator. Used at mid-reschedule sites where a following
  :c:func:`H2K_dosched()` call on the same HW thread will re-arm BESTWAIT.

* **Arming variants** (:c:func:`H2K_ready_append_arm`, :c:func:`H2K_ready_insert_arm`,
  :c:func:`H2K_ready_remove_arm`) — manipulate the ready list AND arm the hardware
  BESTWAIT comparator with the new best ready priority. Used at wake/sleep-transition
  sites where the thread is switched to immediately, or at :c:func:`H2K_ready_getbest()`
  to ensure BESTWAIT is always re-armed after removing the best thread.

The BESTWAIT comparator is a hardware mechanism that raises a reschedule interrupt
whenever any HW thread's STID.PRIO is strictly worse (higher number) than the armed
BESTWAIT value. 

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

.. c:function:: void H2K_ready_init()

Description
~~~~~~~~~~~

Initializes the H2K_kg.ready structures.

Functionality
~~~~~~~~~~~~~

Set all elements of H2K_kg.ready to NULL, and set H2K_kg.ready_valids to zero.


H2K_ready_best_prio
-------------------

.. c:function:: static inline u32_t H2K_ready_best_prio()

	:returns: the priority corresponding to the ready thread with the best priority.
		Returns a value of MAX_PRIOS or higher if no threads are ready.

Description
~~~~~~~~~~~

Functionality
~~~~~~~~~~~~~

Count Trailing Zeros of H2K_kg.ready_valids.



H2K_ready_any_valid
-------------------

.. c:function:: static inline u32_t H2K_ready_any_valid()

	:returns: whether any threads are ready.

Description
~~~~~~~~~~~

Functionality
~~~~~~~~~~~~~

Tests whether :c:func:`H2K_ready_best_prio()` returns MAX_PRIOS or higher.



H2K_ready_prio_valid
--------------------

.. c:function:: static inline u32_t H2K_ready_prio_valid(u32_t prio)

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

.. c:function:: static inline void H2K_ready_set_prio(u32_t prio)

	:param prio: the priority to set

Description
~~~~~~~~~~~

Sets a given priority as valid.

Functionality
~~~~~~~~~~~~~

Set the bit in ready_valids corresponding to the priority.



H2K_ready_clear_prio
--------------------

.. c:function:: static inline void H2K_ready_clear_prio(u32_t prio)

	:param prio: the priority to clear

Description
~~~~~~~~~~~

Sets a given priority as invalid.

Functionality
~~~~~~~~~~~~~

Clear the bit in ready_valids corresponding to the priority.



H2K_ready_append
----------------

.. c:function:: static inline void H2K_ready_append(H2K_thread_context *thread)

	:param thread: the thread to add

Description
~~~~~~~~~~~

Appends the thread to the ready list.

Functionality
~~~~~~~~~~~~~

We get the priority from the thread context.  Next, we call :c:func:`H2K_ring_append()`
on the H2K_kg.ready ring at the thread's priority.  Finally, we call
:c:func:`H2K_ready_set_prio()` with the thread's priority.



H2K_ready_insert
----------------

.. c:function:: static inline void H2K_ready_insert(H2K_thread_context *thread)

	:param thread: the thread to add

Description
~~~~~~~~~~~

Inserts the thread to the ready list.

Functionality
~~~~~~~~~~~~~

We get the priority from the thread context.  Next, we call :c:func:`H2K_ring_insert()`
on the H2K_kg.ready ring at the thread's priority.  Finally, we call
:c:func:`H2K_ready_set_prio()` with the thread's priority.



H2K_ready_remove
----------------

.. c:function:: static inline void H2K_ready_remove(H2K_thread_context *thread)

	:param thread: the thread to remove

Description
~~~~~~~~~~~

Removes the thread from the ready list.

Functionality
~~~~~~~~~~~~~

We get the priority from the thread context.  Next, we call :c:func:`H2K_ring_remove()`
on the H2K_kg.ready ring at the thread's priority.  Finally, if there are no
more elements in the ring, we call :c:func:`H2K_ready_clear_prio()` with the
thread's priority.


H2K_ready_append_arm
--------------------

.. c:function:: static inline void H2K_ready_append_arm(H2K_thread_context *thread)

	:param thread: the thread to add

Description
~~~~~~~~~~~

Appends the thread to the ready list and arms the hardware BESTWAIT comparator.

Functionality
~~~~~~~~~~~~~

Calls :c:func:`H2K_ready_append()` to add the thread to the ready list, then
calls :c:func:`H2K_set_bestwait()` with the result of :c:func:`H2K_ready_best_prio()`
to arm the hardware reschedule-interrupt comparator with the best ready priority.

Used at wake/sleep-transition sites where a thread is being placed on the ready 
list and will be switched to immediately or returned without a following :c:func:`H2K_dosched()`
call on the same HW thread. The BESTWAIT arm ensures the hardware comparator is active to
trigger a reschedule interrupt if a running thread's priority becomes worse than
the newly-ready thread's priority.


H2K_ready_insert_arm
--------------------

.. c:function:: static inline void H2K_ready_insert_arm(H2K_thread_context *thread)

	:param thread: the thread to add

Description
~~~~~~~~~~~

Inserts the thread to the ready list and arms the hardware BESTWAIT comparator.

Functionality
~~~~~~~~~~~~~

Calls :c:func:`H2K_ready_insert()` to add the thread to the ready list, then
calls :c:func:`H2K_set_bestwait()` with the result of :c:func:`H2K_ready_best_prio()`
to arm the hardware reschedule-interrupt comparator with the best ready priority.

Used at wake/sleep-transition sites where a thread is being inserted at the head
of the ready list (higher priority within the same priority level) and will be
switched to immediately without a following :c:func:`H2K_dosched()` call.


H2K_ready_remove_arm
--------------------

.. c:function:: static inline void H2K_ready_remove_arm(H2K_thread_context *thread)

	:param thread: the thread to remove

Description
~~~~~~~~~~~

Removes the thread from the ready list and arms the hardware BESTWAIT comparator.

Functionality
~~~~~~~~~~~~~

Calls :c:func:`H2K_ready_remove()` to remove the thread from the ready list, then
calls :c:func:`H2K_set_bestwait()` with the result of :c:func:`H2K_ready_best_prio()`
to arm the hardware reschedule-interrupt comparator with the best ready priority.

Used in :c:func:`H2K_ready_getbest()` to ensure BESTWAIT is always re-armed after
removing the best thread from the ready list. Also used in :c:func:`H2K_resched()`
to immediately close the disarmed window after a hardware reschedule interrupt
fires (which resets BESTWAIT to 0x1FF).


H2K_ready_getbest
-----------------

.. c:function:: static inline H2K_thread_context *H2K_ready_getbest()

	:returns: the best priority ready thread, which has been removed from
		the ready list.  Or, if no threads are ready, returns NULL.

Description
~~~~~~~~~~~

Removes the best priority thread from the ready list and re-arms BESTWAIT.

Functionality
~~~~~~~~~~~~~

If there are no ready threads (!:c:func:`H2K_ready_any_valid()`), we return NULL.

We call :c:func:`H2K_ready_best_prio()` to obtain the priority of the best priority ready thread.

We then get the thread pointed to by the H2K_kg.ready pointer at the correct priority.

This thread is removed from the ready list by calling :c:func:`H2K_ready_remove_arm()`,
which also re-arms BESTWAIT with the new best priority (or 0x1FF if no threads remain),
and the thread is returned.






Testing
-------

Base Functions (H2K_ready_append, H2K_ready_insert, H2K_ready_remove, H2K_ready_getbest)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Samples
^^^^^^^

* Input: H2K_kg.ready_valids
* Input: H2K_kg.ready array
* Thread to append/insert/remove
* Output: H2K_ready_best_prio: Priority of best ready thread
* Output: H2K_ready_getbest: Best ready thread, removed from readylist, or NULL
* Output: H2K_ready_array / H2K_kg.ready_valids for append/insert/remove/getbest


Important Cases
^^^^^^^^^^^^^^^

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
^^^^^^^

The readylist module is reasonably self-contained, so the test harness will only
use the header file and object file.


BESTWAIT Arming Functions (H2K_ready_append_arm, H2K_ready_insert_arm, H2K_ready_remove_arm)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

The `_arm` variants are tested separately in the H2K_bestwait test suite
(``kernel/data/readylist/tests/H2K_bestwait/test.c``), which validates the
hardware BESTWAIT/SCHEDCFG comparator behavior and the arming discipline.
