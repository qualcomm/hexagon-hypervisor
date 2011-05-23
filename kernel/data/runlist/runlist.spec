
:mod:`runlist` -- managing the currently-running threads
========================================================

.. module:: runlist

runlist and runlist_prios
--------------------------

The ready list contains the threads that are currently running

The requirements for the run list are:

* O(1) insertions of any priority
* O(1) removals of the lowest priority
* Very fast detection of the lowest priority running thread

We acheive this by using an array of linked lists of threads running at a given
priority, and a bitmask that has a bit set for each non-empty linked list.

We insert a thread into the run list by adding the thread to the list at the
priority corresponding to the thread, and setting the corresponding bit.

We find the lowest priority running thread by using the CL0 instruction to find
the lowest priority that has a running thread.  We then can remove the thread in
the corresponding list.

TBD: This datastructure is scalable to any number of hardware threads.  It may
be preferable to have an array of the currently running thread on each hardware
thread, and find the minimum by inspecting each value.


H2K_runlist_init
----------------

.. cfunction:: void H2K_runlist_init()

Description
~~~~~~~~~~~

Initializes the :cdata:`H2K_kg.runlist` and :cdata:`H2K_kg.runlist_prios`
structures.

Functionality
~~~~~~~~~~~~~

Set all elements of :cdata:`H2K_kg.runlist` to NULL, set all elements of
:cdata:`H2K_kg.runlist_prios` to -1.


H2K_runlist_push
----------------

.. cfunction:: static inline void H2K_runlist_push(H2K_thread_context *newthread)

	:param newthread: Thread to add to the runlist

Description
~~~~~~~~~~~

Inserts a thread into the runlist.

Functionality
~~~~~~~~~~~~~

Set newthread's status to running, add newthread to the runlist, and add newthread's
priority to runlist_prios.


H2K_runlist_worst_prio
----------------------

.. cfunction:: static inline u32_t H2K_runlist_worst_prio()

	:returns: the priority of the worst priority running thread.  Returns
		MAX_PRIOS or higher if no threads are in the runlist.

Description
~~~~~~~~~~~

Returns the priority corresponding to the running thread with the worst priority.
Returns MAX_PRIOS or higher if no threads are in the runlist.

Functionality
~~~~~~~~~~~~~

We iterate through the hardware threads to find the one with the worst priority
and return its priority.


H2K_runlist_worst_prio_hthread
------------------------------

.. cfunction:: static inline u32_t H2K_runlist_worst_prio_hthread()

	:returns: the hardware thread number of the worst priority running thread.
                Returns -1 if no threads are in the runlist.

Description
~~~~~~~~~~~

Returns the priority corresponding to the running thread with the worst priority.
Returns MAX_PRIOS or higher if no threads are in the runlist.

Functionality
~~~~~~~~~~~~~

We iterate through the hardware threads to find the one with the worst priority
and return its priority.


H2K_runlist_remove
------------------

.. cfunction:: static inline void H2K_runlist_remove(H2K_thread_context *thread)

	:param thread: the thread to remove from the runlist

Description
~~~~~~~~~~~

Removes ``thread`` from the runlist.  By calling this function, you guarantee
that the thread is in the runlist.

Functionality
~~~~~~~~~~~~~

We set runlist to NULL and runlist_prios to -1 for thread's hthread.





Testing
-------

Samples
~~~~~~~

* Input: H2K_kg.runlist array
* Input: H2K_kg.runlist_prios array
* Thread to push/remove
* Output: H2K_runlist_worst_prio: Priority of worst running thread
* Output: H2K_ready_array / H2K_ready_valids for remove/push


Important Cases
~~~~~~~~~~~~~~~

* H2K_runlist_worst_prio and H2K_runlist_worst_prio_hthread when the runlist is empty
* H2K_runlist_worst_prio and H2K_runlist_worst_prio_hthread when the runlist has
  different threads in it

* H2K_runlist_push should set H2K_kg.runlist and H2K_kg.runlist_prios appropriately.

* H2K_runlist_remove should set H2K_kg.runlist to NULL and H2K_kg.runlist_prios to an
  invalid priority.

* Check H2K_runlist_init clears out randomized values.

Harness
~~~~~~~

The runlist module is reasonably self-contained, so the test harness will only
use the header file and object file.  



