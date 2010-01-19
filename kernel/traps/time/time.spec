
:mod:`time` -- cycle count tracking information
================================================

.. module:: time


H2K_cputime_get
---------------

.. cfunction:: u64_t H2K_cputime_get(H2K_thread_context *me)

	:param me: Pointer to the thread context of the currently-executing thread
	:returns: Current total time the thread has been scheduled, in processor cycles.

Description
~~~~~~~~~~~

Returns the total time the current thread has been scheduled.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

Returns the me->totalcycles plus the difference between the current pcycle
count and me->oncpu_start.


H2K_pcycles_get
---------------

.. cfunction:: u64_t H2K_pcycles_get(H2K_thread_context *me)


	:param me: Pointer to the thread context of the currently-executing thread
	:returns: The processor cycle count.


Description
~~~~~~~~~~~

Returns the processor cycle count

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

Returns the current pcycle count


Testing
-------

Samples
~~~~~~~

* Pcycle value
* Whether the thread has been scheduled or not

Important Cases
~~~~~~~~~~~~~~~

* Pcycle count should be correct
* CPU time should not increment while thread is not scheduled
* CPU time should increment while thread is scheduled

Harness
~~~~~~~

H2 is started in the normal method.

We spawn two threads.

The first thread checks that subsequent calls to H2K_pcycles_get have similar
increasing values.

We then check that subsequent calls to H2K_cputime_get have similar increasing
values.

We check to make sure that after spinning for a large number of cycles, that 
the cputime and pcycles have expected values.

Finally, we block while the second thread spins for a large number of cycles.
When the second thread is finished, the first thread is unblocked.  We check
the pcycles and cputime before and after blocking, and expect a large delta
in the cputime, approximately as large as the number of cycles we spin in the
second thread.


