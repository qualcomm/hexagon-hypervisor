
MODULE: thread_id

FUNCTION: u32_t BLASTK_thread_id(thread_context *me)

DESCRIPTION:

The BLASTK_thread_id function returns a unique id for the requesting thread.

INPUT:

Argument 0: Pointer to the current context

OUTPUT:

A unique ID for the running thread

FUNCTIONALITY:

Each thread should have a unique ID.

We currently use the pointer to the thread context as the Thread ID.

This helps us change priority of other threads.

Alternatively, if thread contexts must be contiguous in virtual memory, we
could have an index.

Alternatively, we could have a globally-incrementing counter.  However, it
makes it difficult to map id->pointer.  On the other hand, this is rarely
used (currently, mainly for changing priority of another thread).


