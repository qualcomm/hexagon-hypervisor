
:mod:`max` -- compile-time definitions
=========================================

.. module:: max

This unit contains compile-time definitions for C and Assembly that 
are needed over several units.

DEFINITION: `MAX_HTHREADS`
--------------------------

Maximum number of hardware threads that can be turned on

DEFINITION: `MAX_THREADS`
--------------------------

Number of software threads that are available without additional
runtime configuration.

DEFINITION: `KERNEL_STACK_SIZE`
--------------------------

Size of the per-hardware-thread kernel stack

DEFINITION: `MAX_PRIOS`
--------------------------

Maximum number of priorities, up to 32.  For more than 32 priorities, several
data structures will need to be changed.

DEFINITION: `RESCHED_INT`
--------------------------

The number of the interrupt reserved for the reschedule interrupt at boot
time.  This can be reconfigured at runtime.

DEFINITION: `SSR_DEFAULT`
--------------------------

The SSR value of the initial thread to execute.

