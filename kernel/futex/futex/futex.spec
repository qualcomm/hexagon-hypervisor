
:mod:`futex` -- Generic Blocking / Unblocking Services
======================================================

.. module:: futex


Testing
-------


Samples
~~~~~~~

* FUTEX_HASHSIZE - define (default 6)
* lock - pointer to futex

n_to_wake (H2K_futex_resume)

H2K_futex_wait return value
H2K_futex_resume return value

Interesting Cases
~~~~~~~~~~~~~~~~~

Lower 12 bits of futex address all the same
Lower 12 bits of futex address all different

Wait on a valid lock
Wait on an invalid lock

Wake 0 threads (via n_to_wake == 0, or n_to_wake > 0 with no threads ready)
Wake 1 thread
Wake multiple threads


PI raising priority
PI not raising priority (and not lowering)
PI handoff to waiting thread
PI handoff to no thread waiting

Harness
~~~~~~~

Testcases will instantiate the full kernel and make use of multiple software threads.

Future tests may test the hashing function more thoroughly


