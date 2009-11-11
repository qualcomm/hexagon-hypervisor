
MODULE: runlist


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


FUNCTION: void BLASTK_runlist_init()

DESCRIPTION:

Initializes the BLASTK_runlist structures.

INPUTS:

OUTPUTS:

FUNCTIONALITY:

Set all elements of BLASTK_runlist to NULL, and set BLASTK_runlist_valids to zero.



FUNCTION: static inline void BLASTK_runlist_push(BLASTK_thread_context *newthread)

DESCRIPTION:

Inserts a thread into the runlist.

INPUTS:

Argument 0: Thread to add to the runlist

OUTPUTS:

FUNCTIONALITY:

The specified thread is added to the head of the runlist at the correct priority.

The bit in runlist_valids corresponding to the priority is set.



FUNCTION: static inline int BLASTK_runlist_worst_prio()

DESCRIPTION:

Returns the priority corresponding to the running thread with the worst priority.

INPUTS:

OUTPUTS:

Returns the priority of the worst priority running thread.

FUNCTIONALITY:

We count leading zeros of BLASTK_runlist_valids.  This gives us the number of 
lowest priorities that have no threads.  We subtract this value from the priority
corresponding to the most significant bit.



FUNCTION: static inline void BLASTK_runlist_remove(BLASTK_thread_context *thread)

DESCRIPTION:

Removes ``thread`` from the runlist.  By calling this function, you guarantee
that the thread is in the runlist.

INPUTS:

Argument 0: the thread to remove from the runlist

OUTPUTS:

FUNCTIONALITY:

We get the priority from the thread structure.  We then look at the runlist linked
list at the corresponding priority.  When we find the thread, we remove it from the
linked list.  If the runlist has emptied because of the removal of the thread, we
clear the bit from BLASTK_runlist_valids.


