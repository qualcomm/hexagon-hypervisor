
:mod:`lowprio` -- managing the lowest priority thread
=====================================================

.. module:: lowprio

H2K_lowprio_notify
------------------

.. cfunction:: static inline void H2K_lowprio_notify()

Description
~~~~~~~~~~~

:cfunc:`H2K_lowprio_notify()` identifies a new thread to be the lowest-priority thread
for receiving interrupts, and takes action to make that thread be receptive to
interrupts.

Functionality
~~~~~~~~~~~~~

First, we get the priority of the worst priority running thread.  We look at the 
H2K_runlist entry at that priority, which points to the thread that was inserted
at the worst priority most recently.  We then look at the hthread field of this 
thread, and use that to set the H2K_priomask bit corresponding to the 
hardware thread and to call thread_prio_change_low().


H2K_lowprio_raise
-----------------

.. cfunction:: static inline void H2K_lowprio_raise()

Description
~~~~~~~~~~~

:cfunc:`H2K_lowprio_raise()` takes the thread currently marked as lowest priority, and 
modifies the state to indicate it is no longer lowest priority.

Functionality
~~~~~~~~~~~~~

If H2K_wait_mask is nonzero, we return, as we should never mask interrupts on 
a waiting thread.  Otherwise, count the trailing zeros of H2K_priomask, which
yields the hardware thread that should no longer be the low priority thread.  
We clear that bit from the H2K_priomask and call :cfunc:`H2K_prio_change_high()` for
the hardware thread.  


H2K_lowprio_raise
-----------------

.. cfunction:: static inline void H2K_lowprio_init()

Description
~~~~~~~~~~~

:cfunc:`H2K_lowprio_init()` initializes the data structures used by the lowprio facility.

Functionality
~~~~~~~~~~~~~

Set H2K_wait_mask and H2K_priomask to zero.



Testing
-------


Samples
~~~~~~~

* Input: runlist
* Input: H2K_wait_mask
* I/O: H2K_priomask
* Output: IMASK values

Important Cases
~~~~~~~~~~~~~~~

* H2K_wait_mask == 0: H2K_lowprio_raise should have no effect

Harness
~~~~~~~

H2 lib kernel will be built.

Various threads at various priorities should be added to runlist.
The lowest priority thread in the runlist should be marked as lowprio.
H2K_lowprio_raise will be called, and the formerly lowest priority
hardware thread should have a modified IMASK and the H2K_priomask bit
should be configured to be non-receptive to most interrutps.

H2K_lowprio_notify should then be called.  If H2K_wait_mask is nonzero, the
lowest priority thread in the runlist should be selected, the corresponding bit
should be added to H2K_priomask, and the IMASK on the corresponding hardware
thread should be receptive to most interrupts.

Also, check that H2K_lowprio_init initializes H2K_wait_mask and H2K_priomask.


