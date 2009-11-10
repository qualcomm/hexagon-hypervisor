
MODULE: sched_yield

FUNCTION: void sched_yield(thread_context *me)

DESCRIPTION:

The sched_yield function yields execution to all other threads at the same priority.

INPUTS:

Argument 0: Pointer to the current thread context

OUTPUTS:

None

FUNCTIONALITY:

If the readylist at the current priority is empty, the sched_yield function may
return immediately.

Otherwise, we acquire the Big Kernel Lock.

Next, the sched_yield function removes the current thread from the runlist, and
appends it on the end of the ready list.  

We can then call dosched to pick a new thread to run.

As an optimization, we can instead remove the thread at the head of the
readylist at the same priority, and insert it into the runlist.  We switch to
the thread inserted into the runlist.

