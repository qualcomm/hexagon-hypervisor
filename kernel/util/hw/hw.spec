
:mod:`hw` -- hardware interface intrinsics
==========================================

.. module:: hw

This unit contains intrinsics which implement interfaces
to hardware.


ciad
----

.. cfunction:: static inline void ciad(u32_t mask)

	:param mask: A mask of which interrupts to clear.  It is
		implementation-defined on which interrupts in the mask correspond to which
		hardawre interrupts.

Description
~~~~~~~~~~~

Clears the Interrupt Auto Disable register using the specified mask.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

We invoke the CIAD instruction with the designated mask.


change_imask
------------

.. cfunction:: static inline void change_imask(u32_t thread, u32_t imask)

	:param thread: The thread to change
	:param imask: The IMASK value to write

Description
~~~~~~~~~~~

Changes the IMASK register for the specified thread to ``imask``

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

The destination thread is put in a predicate register, and then we use the
SETIMASK instruction to modify the IMASK for the thread.


get_imask
---------

.. cfunction:: static inline u32_t get_imask(u32_t thread)

	:param thread: The thread to fetch the imask of

Description
~~~~~~~~~~~

Get the IMASK register for the specified thread

Input
~~~~~

Output
~~~~~~

Returns the imask of the desired hardware thread

Functionality
~~~~~~~~~~~~~

The source thread is put in a predicate register, and then we use the
GETIMASK instruction to retrieve the IMASK.


resched_int
-----------

.. cfunction:: static inline void resched_int()

Description
~~~~~~~~~~~

This function raises the reschedule interrupt.

Input
~~~~~


Output
~~~~~~


Functionality
~~~~~~~~~~~~~

The constant RESCHED_INT_INTMASK is placed into a register, and the SWI
instruction is invoked with that register.

highprio_imask
--------------

.. cfunction:: static inline void highprio_imask(u32_t hthread)

	:param hthread: The hardware thread 

Description
~~~~~~~~~~~

Changes the IMASK of the current thread to be appropriate for a non-low-priority thread.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

We form the appropriate IMASK value for the current hardware thread to mask off
most of the interrupts, and then place that value in IMASK.




lowprio_imask
------------

.. cfunction:: static inline void lowprio_imask(u32_t hthread)

	:param hthread: The hardware thread 

Description
~~~~~~~~~~~

Changes the IMASK of the current thread to be appropriate for a low-priority thread.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

We form the appropriate IMASK value for the current hardware thread to enable
most of the interrupts, and then place that value in IMASK.


get_ssr
-------

.. cfunction:: static inline u32_t get_ssr()

Description
~~~~~~~~~~~

Returns the value of the SSR register.

Input
~~~~~

Output
~~~~~~

Returns the value of the SSR register

Functionality
~~~~~~~~~~~~~

The SSR value is transfered to a temporary value that is returned.



get_hwtnum
----------

.. cfunction:: static inline u32_t get_hwtnum()

Description
~~~~~~~~~~~

Returns the current hardware thread number.

Input
~~~~~

Output
~~~~~~

Returns the current hardware thread number.

Functionality
~~~~~~~~~~~~~

For V3 and earlier, we extract the TNUM field from SSR.  For V4, we use the HTNUM register.



H2K_mutex_lock_k0
-----------------

.. cfunction:: static inline void H2K_mutex_lock_k0()

AVAILABILITY(ARCH >= 3)

Description
~~~~~~~~~~~

Lock the K0 hardware lock.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

This merely executes the K0LOCK instruction.




H2K_mutex_unlock_k0
--------------------

.. cfunction:: static inline void H2K_mutex_unlock_k0()

AVAILABILITY(ARCH >= 3)

Description
~~~~~~~~~~~

Unlock the K0 hardware lock.

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

This merely executes the K0UNLOCK instruction.


H2K_clear_gie
-------------

.. cfunction::  static inline void H2K_clear_gie()

Description
~~~~~~~~~~~

This function clears the global interrupt enable bit in SYSCFG, turning all interrupts off.


H2K_set_gie
-----------

.. cfunction::  static inline void H2K_set_gie()

Description
~~~~~~~~~~~

This function sets the global interrupt enable bit in SYSCFG.


H2K_get_ipend
-------------

.. cfunction::  static inline u32_t H2K_get_ipend()

Description
~~~~~~~~~~~

This function retrieves the contents of the IPEND status register.


H2K_clear_ipend
---------------

.. cfunction::  static inline void H2K_clear_ipend(u32_t mask)

	:param mask:  Mask of bits to clear out of IPEND

Input
~~~~~

Description
~~~~~~~~~~~

This function retrieves clears the contents of the IPEND status register.



