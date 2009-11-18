
MODULE: readylist

DATA: BLASTK_ready, BLASTK_ready_valids

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

FUNCTION: void BLASTK_ready_init()

DESCRIPTION:

Initializes the BLASTK_ready structures.

INPUTS:

OUTPUTS:

FUNCTIONALITY:

Set all elements of BLASTK_ready to NULL, and set BLASTK_ready_valids to zero.



FUNCTION: static inline u32_t BLASTK_ready_best_prio()

DESCRIPTION:

Returns the priority corresponding to the ready thread with the best priority.

INPUTS:

OUTPUTS:

FUNCTIONALITY:

Count Trailing Zeros of BLASTK_ready_valids.


FUNCTION: static inline void BLASTK_ready_append(BLASTK_thread_context *thread)

DESCRIPTION:

Appends the thread to the ready list.

INPUTS:

Argument 0: the thread to add

OUTPUTS:

FUNCTIONALITY:

We get the priority from the thread context.  Next, we call BLASTK_ring_append
on the BLASTK_ready ring at the thread's priority.  Finally, we set the bit of
BLASTK_ready_valids at the thread's priority.


FUNCTION: static inline void BLASTK_ready_insert(BLASTK_thread_context *thread)

DESCRIPTION:

Inserts the thread to the ready list.

INPUTS:

Argument 0: the thread to add

OUTPUTS:

FUNCTIONALITY:

We get the priority from the thread context.  Next, we call BLASTK_ring_insert
on the BLASTK_ready ring at the thread's priority.  Finally, we set the bit of
BLASTK_ready_valids at the thread's priority.


FUNCTION: static inline void BLASTK_ready_remove(BLASTK_thread_context *thread)

DESCRIPTION:

Removes the thread from the ready list.

INPUTS:

Argument 0: the thread to remove

OUTPUTS:

FUNCTIONALITY:

We get the priority from the thread context.  Next, we call BLASTK_ring_remove
on the BLASTK_ready ring at the thread's priority.  Finally, if there are no
more elements in the ring, we clear the bit of BLASTK_ready_valids at the
thread's priority.


FUNCTION: static inline BLASTK_thread_context *BLASTK_ready_getbest()

DESCRIPTION:

Removes the best priority thread from the ready list.

INPUTS:

OUTPUTS:

Returns the best priority ready thread, which has been removed from the ready list.
Returns NULL if no threads are ready.

FUNCTIONALITY:

If there are no ready threads, we return NULL.

We call BLASTK_ready_best_prio() to obtain the priority of the best priority ready thread.

We then get the thread pointed to by the BLASTK_ready pointer at the correct priority.

This thread is removed from the ready list by calling BLASTK_ready_remove, and returned.

