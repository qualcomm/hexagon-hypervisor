
:mod:`runlist` -- managing the currently-running threads
=========================================================

.. module:: runlist

The ready list contains the threads that are currently running

The requirements for the run list are:

* O(1) insertions of any priority
* O(1) removals of the lowest priority
* Very fast detection of the lowest priority running thread

We acheive this by using an array of linked lists of threads running at a given
priority, and a bitmask that has a bit set for each non-empty linked list.

We insert a thread into the run list by adding the thread to the list at the
priority corresponding to the thread, and setting the corresponding bit.

We find the lowest priority running thread by using the CT0 instruction to find
the lowest priority that has a running thread.  We then can remove the thread in
the corresponding list.

TBD: This datastructure is scalable to any number of hardware threads.  It may
be preferable to have an array of the currently running thread on each hardware
thread, and find the minimum by inspecting each value.



H2K_runlist_init
----------------

..cfunction:: void H2K_runlist_init()

Description
~~~~~~~~~~~

Initializes the H2K_runlist structures.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

Set all elements of H2K_runlist to NULL, and set H2K_runlist_valids to zero.


H2K_runlist_push
----------------

.. cfunction:: static inline void H2K_runlist_push(H2K_thread_context *newthread)

	:param newthread: Thread to add to the runlist

Description
~~~~~~~~~~~

Inserts a thread into the runlist.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

The specified thread is added to the head of the runlist at the correct priority.

The bit in runlist_valids corresponding to the priority is set.


H2K_runlist_worst_prio
----------------------

.. cfunction:: static inline u32_t H2K_runlist_worst_prio()

	:returns: the priority of the worst priority running thread.  Returns
		MAX_PRIOS or higher if no threads are in the runlist.

Description
~~~~~~~~~~~

Returns the priority corresponding to the running thread with the worst priority.
Returns MAX_PRIOS or higher if no threads are in the runlist.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

We count leading zeros of H2K_runlist_valids.  This gives us the number of 
lowest priorities that have no threads.  We subtract this value from the priority
corresponding to the most significant bit.


H2K_runlist_remove
------------------

.. cfunction:: static inline void H2K_runlist_remove(H2K_thread_context *thread)

	:param thread: the thread to remove from the runlist

Description
~~~~~~~~~~~

Removes ``thread`` from the runlist.  By calling this function, you guarantee
that the thread is in the runlist.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

We get the priority from the thread structure.  We then look at the runlist linked
list at the corresponding priority.  When we find the thread, we remove it from the
linked list.  If the runlist has emptied because of the removal of the thread, we
clear the bit from H2K_runlist_valids.





Testing
-------

Samples
~~~~~~~

* Input: H2K_runlist_valids
* Input: H2K_runlist array
* Thread to push/remove
* Output: H2K_runlist_worst_prio: Priority of worst running thread
* Output: H2K_ready_array / H2K_ready_valids for remove/push


Important Cases
~~~~~~~~~~~~~~~

* H2K_runlist_worst_prio when the runlist is empty
* H2K_runlist_worst_prio when the runlist has different threads in it

* H2K_runlist_push a thread into a runlist with no other threads at the same
  priority; H2K_runlist_valids bit should be set
* H2K_runlist_push a thread into a runlist with other threads at the same
  priority; H2K_runlist_valids bit should remain set

* H2K_runlist_remove a thread from the runlist with only the specified thread at
  the priority, H2K_runlist_valids bit should be cleared
* H2K_runlist_remove a thread from the runlist with more than the specified thread at
  the priority, H2K_runlist_valids bit should remain set

* Check H2K_runlist_init clears out randomized values.

Harness
~~~~~~~

The runlist module is reasonably self-contained, so the test harness will only
use the header file and object file.  



