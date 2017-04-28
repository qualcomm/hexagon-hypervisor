:mod:`vmfuncs` -- Various VM Functionalities
============================================

.. module:: vmfuncs

H2K_vmtrap_version
------------------

.. c:function:: void H2K_vmtrap_version(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

This function handles the "vmversion" virtual instruction, to select/identify
VM architecture versions.

Functionality
~~~~~~~~~~~~~

This function sets r0 to the only supported virtual machine version.

H2K_vmtrap_return
-----------------

.. c:function:: void H2K_vmtrap_return(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

This function handles the "vmreturn" virtual instruction, to return
from an event.

Functionality
~~~~~~~~~~~~~

This function uses the Guest registers GELR, GSSR, and (potentially) GOSP to
switch from guest mode back to either guest or user mode.

If the IE (interrupt enable), SS (single step), or UM (User Mode) bits are set,
the appropriate settings are made to the virtual machine state, including
values desinted for the Hexagon system architecture registers.

If interrupts are being enabled, we call :c:func:`H2K_enable_guest_interrupts()` 
to handle the change.


H2K_vmtrap_setvec
-----------------

.. c:function:: void H2K_vmtrap_setvec(H2K_thread_context *me)

	:param me: Pointer to the current thread context


Description
~~~~~~~~~~~

This function sets the guest event vector base to the specified value.

Functionality
~~~~~~~~~~~~~

Change the GEVB in the thread context to the value in R00, and set r00
to zero for success.

H2K_vmtrap_setie
----------------

.. c:function:: void H2K_vmtrap_setie(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

This function enables or disables interrupts, returning the previous value in r0.

Functionality
~~~~~~~~~~~~~

First, we get the current interrupt enabled state.  We then call 
:c:func:`H2K_enable_guest_interrupts()` or :c:func:`H2K_disable_guest_interrupts()` 
depending on the value of r0.  Finally, we set r00 to the former interrupt 
state and return.

H2K_vmtrap_getie
----------------

.. c:function:: void H2K_vmtrap_getie(H2K_thread_context *me)

	:param me: Pointer to the current thread context


Description
~~~~~~~~~~~

This function returns the current state of interrupts, enabled or disabled.

Functionality
~~~~~~~~~~~~~

Returns the IE status for the virtual CPU in the context r0.

H2K_vmtrap_get_pcycles
----------------------

.. c:function:: void H2K_vmtrap_get_pcycles(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Returns the cpu time for the current virtual CPU in r1:0, and the hardware PMU counters in r3:2.

Functionality
~~~~~~~~~~~~~

This function uses :c:func:`H2K_cputime_get()` to get the CPU time, places
the result in the context r0100, and copies pcycle registers to r0302.

H2K_vmtrap_set_pcycles
----------------------

.. c:function:: void H2K_vmtrap_set_pcycles(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

This function sets the cpu time for the current virtual CPU.

Functionality
~~~~~~~~~~~~~

This function resets the oncpu_start value for the current hardware thread to 
the current pcycle count, and sets the totalcycles value for the current
thread context to the value specified in r0100.

NOTE: This function should be deprecated.  The guest can keep track of a delta itself.




H2K_vmtrap_wait
---------------

.. c:function:: void H2K_vmtrap_wait(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

This function waits for an interrupt.

Functionality
~~~~~~~~~~~~~

If an interrupt is pending, the function returns immediately with the interrupt
that was pending.  Otherwise, it blocks until an interrupt is enabled.

We keep track of the VCPUs that are waiting so that they can be favored for
wakeup if shared interrupts come in.

H2K_vmtrap_yield
----------------

.. c:function:: void H2K_vmtrap_yield(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Yields to another VCPU at the current priority.

Functionality
~~~~~~~~~~~~~

Calls :c:func:`H2K_sched_yield()` and puts 0 for success in r00.

H2K_vmtrap_start
----------------

.. c:function:: void H2K_vmtrap_start(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Starts another VCPU.

Functionality
~~~~~~~~~~~~~

Calls :c:func:`H2K_thread_create_no_squash()` using r0 as the start address, r1
as the stack pointer, and with the same priority as the calling VCPU.

H2K_vmtrap_stop
---------------

.. c:function:: void H2K_vmtrap_stop(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Terminates the current thread and makes it ready for reallocation by 
:c:func:`H2K_vmtrap_start()`.

Functionality
~~~~~~~~~~~~~

Calls :c:func:`H2K_thread_stop()`.

H2K_vmtrap_vmpid
----------------

.. c:function:: void H2K_vmtrap_vmpid(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Returns a unique ID for the current Virtual CPU.

Functionality
~~~~~~~~~~~~~

Places the ID into r00.

H2K_vmtrap_setregs
------------------

.. c:function:: void H2K_vmtrap_setregs(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Sets the Guest registers g0-g3 with the values in r0-r3.

Functionality
~~~~~~~~~~~~~

Sets GELR with r0, GSSR with r1, GOSP with r2, and GBADVA with r3.

H2K_vmtrap_getregs
------------------

.. c:function:: void H2K_vmtrap_getregs(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Sets the registers r0-r3 with the values in Guest registers g0-g3.

Functionality
~~~~~~~~~~~~~

Sets r0 with GELR, r1 with GSSR, r2 with GOSP, and r3 with GBADVA.

H2K_vmtrap_pmuctrl
------------------

.. c:function:: void H2K_vmtrap_pmuctrl(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Call H2K_trap_pmuctrl()

Functionality
~~~~~~~~~~~~~

If r0 == PMUCTRL_THREADSET (enable/disable PMU for CPU) and r1 == 0 (thread
ID 0) then call :c:func:`H2K_trap_pmuctrl()` with the given arguments for each
thread ID in the current vmblock.  Return the number of successful calls in r0.

Else return the result of :c:func:`H2K_trap_pmuctrl()` in r0.

H2K_vmtrap_info
---------------

.. c:function:: void H2K_vmtrap_info(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

Call H2K_trap_info()

Functionality
~~~~~~~~~~~~~
Return the result of :c:func:`H2K_trap_info()` in r0;

Testing
-------

Call the various functions and check expected values.

The most interesting functions are:

* :c:func:`H2K_vmtrap_return()`
* :c:func:`H2K_vmtrap_wait()`
* :c:func:`H2K_vmtrap_setie()`

These functions have the most complexity and need to manipulate registers or
other state in interesting ways.

