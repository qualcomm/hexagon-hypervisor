
MODULE: thread_stop

FUNCTION: void BLASTK_thread_stop(thread_context *me)

The BLASTK_thread_stop function terminates the thread.

INPUT:

Argument 0: The pointer to the current thread context

OUTPUT:

None, does not return

FUNCTIONALITY:

First, we acquire the BKL.

Next, we remove the current thread from the runlist.  We then clear the thread
context.  This has the effect of setting the valid field to DEAD.

We then insert the thread into the BLASTK_free_threads list.

Finally, we call BLASTK_dosched() to pick a new thread.  The current thread
should be specified as NULL, rather than as the now-dead thread context pointer.

