
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

