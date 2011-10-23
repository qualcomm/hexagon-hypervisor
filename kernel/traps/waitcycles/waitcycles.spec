
:mod:`waitcycles` -- querying the number of cycles spent in wait mode
=============================================


H2K_waitcycles_get
-----------

.. cfunction:: u64_t  H2K_waitcycles_get(u32_t htid, H2K_thread_context *me)

	:param htid: the hardware thread of interest
	:param me: pointer to the context for the current thread
	:returns: the total cycles the hadware thread has spent in wait mode

Description
~~~~~~~~~~~

Get the total cycles a hadware thread has spent in wait mode

Functionality
~~~~~~~~~~~~~

If the requested hardware thread is not in wait mode, then we read
``H2K_gp->waitcycles[htid]``, and return the resulting value.  If the hardware
thread is in wait mode, then we add the number of cycles that have elapsed since
it last went into wait mode.  If htid is out of range, we return 0.



Testing
-------

Samples
~~~~~~~

* Value in H2K_gp->oncpu_wait
* Value in H2K_gp->waitcycles


Important Cases
~~~~~~~~~~~~~~~

* Getting waitcycles value


Harness
~~~~~~~

Spin for a while while the other hardware threads wait.  Then read the waitcycles
for each hardware thread.  The waitcycles for thread 0 should be close to 0, while
waitcycles for the others should be about the amount of time we spun.
