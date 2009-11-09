
MODULE: thread_stop

FUNCTION: void thread_stop(thread_context *me)

The thread_stop function terminates the thread.

INPUT:

Argument 0: The pointer to the current thread context

OUTPUT:

None, does not return

FUNCTIONALITY:

First, we acquire the BKL.

Next, we remove the current thread from the runlist.  We then clear the thread
context.  This has the effect of setting the valid field to DEAD.

We then insert the thread into the free_threads list.

Finally, we call dosched_with_lock() to pick a new thread.  The current thread
should be specified as NULL, rather than as the now-dead thread context pointer.

