
:mod:`checker` -- Checker functions
===================================

.. module:: checker

This module contains various functions used by standalone tests to
check assumtions.

checker_kernel_locked
---------------------

.. c:function:: void checker_kernel_locked()

Description
~~~~~~~~~~~

If the kernel is locked, return.  Otherwise, trigger an assertion.


checker_ready
-------------

.. c:function:: void checker_ready()

Description
~~~~~~~~~~~

Checks the ready queue for sanity


checker_ring
------------

.. c:function:: s32_t checker_ring(H2K_ringnode_t **x)

           :param x: Pointer to the ring.
           :returns: 0 on success, nonzero on failure

Description
~~~~~~~~~~~

If the specified ring looks sane, return.  Otherwise, trigger an assertion.


checker_ready
-------------

.. c:function:: s32_t checker_ready()

           :returns: 0 on success, nonzero on failure

Description
~~~~~~~~~~~

Checks the run list for sanity, return.  Otherwise, trigger an assertion.

