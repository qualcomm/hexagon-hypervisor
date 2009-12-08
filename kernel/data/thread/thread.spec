
MODULE: thread

This module contains the data structures for threads.

DETAILS:

H2K_free_threads is a linked list of all threads ready for use.


FUNCTION: void H2K_thread_context_clear(H2K_thread_context *thread)

DESCRIPTION:

Zeros a thread context.

INPUTS:

Argument 0: pointer to a thread context

OUPUTS:


FUNCTIONALITY:

Sets every bit in the thread context to zero.

