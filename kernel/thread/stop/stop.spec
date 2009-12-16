
:mod:`thread_stop` -- terminate thread
=======================================

.. module:: thread_stop

H2K_thread_stop
---------------

.. cfunction:: void H2K_thread_stop(thread_context *me)

The H2K_thread_stop function terminates the thread.

Input
~~~~~

Argument 0: The pointer to the current thread context

Output
~~~~~~

None, does not return

Functionality
~~~~~~~~~~~~~

First, we acquire the BKL.

Next, we remove the current thread from the runlist.  We then clear the thread
context.  This has the effect of setting the valid field to DEAD.

We then insert the thread into the H2K_free_threads list.

Finally, we call H2K_dosched() to pick a new thread.  The current thread
should be specified as NULL, rather than as the now-dead thread context pointer.




Testing
-------


Samples
~~~~~~~

* Input: `me` 

Important cases
~~~~~~~~~~~~~~~

* Valid thread as input
* Free thread list is empty
* Free thread list is non-empty

Harness
~~~~~~~

We link directly with the stop object file, and also the runlist object file.

The test harness defines H2K_dosched() to set a flag indicating that the dosched
routine was called.  It also adds test threads correctly into the runlist.

The test harness calls H2K_thread_stop() with a thread context pointer.  We
check to make sure H2K_dosched was called, and that the thread was added to the
free thread list, and that the values were cleared.


