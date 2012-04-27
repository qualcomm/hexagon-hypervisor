
:mod:`timer` -- managing timer hardware
==========================================

.. module:: timer

Timer Management

H2K_timer_int
-------------

.. cfunction:: void H2K_timer_int(u32_t unused, H2K_thread_context *me, u32_t hwtnum)

	:param unused: Unused parameter
	:param me: pointer to the thread that was interrupted
	:param hwtnum: Hardware thread currently executing


Description
~~~~~~~~~~~

This function is called when the timer interrupt triggers

Functionality
~~~~~~~~~~~~~

Expire all timers before or near the current time.  Update the current time.
Reprogram the timer device for the next expiry.

H2K_timer_trap
--------------

.. cfunction:: u64_t H2K_timer_trap(u32_t traptype, u64_t arg, H2K_thread_context *me)

	:param traptype: Which trap to do
	:param arg: For setting a timeout, the "nsec" absolute time of the timeout
	:param me: Pointer to the current context

Description
~~~~~~~~~~~

If an invalid request is made, -1 is returned.  

Otherwise, this function calls a handler function.

0) Get Frequency
1) Get Resolution
2) Get Rough Time
3) Get Time
4) Get Timeout
5) Set Timeout
6) Set Active Timeout to Sooner Time

Functionality
~~~~~~~~~~~~~

We use a look up table of pointers to handler functions, described below.


H2K_timer_get_freq
~~~~~~~~~~~~~~~~~~

.. cfunction:: u64_t H2K_timer_get_freq(u32_t unused, u64_t unused2, H2K_thread_context *me)

	:param unused: not used
	:param unused2: not used
	:param me: pointer to the current context

Description
~~~~~~~~~~~

This function returns the exact number of "nsecs" per second.  If exact time is needed,
this can be used to compensate if nsecs are not exactly nanoseconds.


H2K_timer_get_resolution
~~~~~~~~~~~~~~~~~~~~~~~~

.. cfunction:: u64_t H2K_timer_get_resolution(u32_t unused, u64_t unused2, H2K_thread_context *me)

	:param unused: not used
	:param unused2: not used
	:param me: pointer to the current context

Description
~~~~~~~~~~~

This function returns the resolution of the timer.  All nsec times in a resolution-sized
chunk are equivalent times.

Note that this does not describe any guarantee of when the timer expiry will be observed
with respect to the time, however expiration accuracy better than the
granularity is impossible.

Additionally, it will be observed that calls to :cfunc:`H2K_timer_get_time()` are always
a multiple of the granularity.


H2K_timer_get_time
~~~~~~~~~~~~~~~~~~

.. cfunction:: u64_t H2K_timer_get_time(u32_t unused, u64_t unused2, H2K_thread_context *me)

	:param unused: not used
	:param unused2: not used
	:param me: pointer to the current context
	:returns: The current time

Description
~~~~~~~~~~~

Returns the number of nsecs since boot


H2K_timer_get_roughtime
~~~~~~~~~~~~~~~~~~~~~~~

.. cfunction:: u64_t H2K_timer_get_roughtime(u32_t unused, u64_t unused2, H2K_thread_context *me)

	:param unused: not used
	:param unused2: not used
	:param me: pointer to the current context
	:returns: An approximation of the time


Description
~~~~~~~~~~~

Gives an approximation of the time based on the last time the timer hardware
was interacted with, but does not query the actual timer hardware.  This is
useful if the timer hardware is extremely slow.

H2K_timer_get_timeout
~~~~~~~~~~~~~~~~~~~~~

.. cfunction:: u64_t H2K_timer_get_timeout(u32_t unused, u64_t unused2, H2K_thread_context *me)

	:param unused: not used
	:param unused2: not used
	:param me: pointer to the current context
	:returns: The time of the timer expiration, or 0 if no timer is active

Description
~~~~~~~~~~~

If no timeout is active, returns 0.  Otherwise, returns the time of the next timeout.


H2K_timer_set_timeout
~~~~~~~~~~~~~~~~~~~~~

.. cfunction:: u64_t H2K_timer_set_timeout(u32_t unused, u64_t timeout, H2K_thread_context *me)

	:param unused: not used
	:param timeout: Requested time for timeout
	:param me: pointer to the current context
	:returns: The time of the set timeout, or 0 if the timeout can not be set


Description
~~~~~~~~~~~

This function changes the timeout request.

If a previous timeout is active, it is disabled.

If the new timeout request time is acceptable, the timeout is activated and the
corresponding time is returned.

If the new timeout request time is not acceptable, 0 is returned and no timeout 
is activated.

Request times can be invalid for the following reasons:
	* The time is in the past
	* The time is too close to the present to activate a timer
	* The time is too far in the future to ever expire

Timeouts can be disactivated by requesting an invalid time, such as the time 0.


H2K_timer_init
~~~~~~~~~~~~~~


.. cfunction:: void H2K_timer_init()

Description
~~~~~~~~~~~

This function initializes the timer state.

H2K_timer_dotimeout
~~~~~~~~~~~~~~~~~~~



