
MODULE: readylist


The ready list contains the threads that are ready for execution.

The requirements for the ready list are:

* O(1) insertions of any priority
* O(1) removals of the highest priority
* Very fast detection of the highest priority ready thread

We acheive this by using an array of linked lists of threads ready at a given
priority, and a bitmask that has a bit set for each non-empty linked list.

We insert a thread into the ready list by adding the thread to the list at the
priority corresponding to the thread, and setting the corresponding bit.

We find the highest priority ready thread by using the CL0 instruction to find
the highest priority that has a ready thread.  We then can remove the thread in
the corresponding list.


