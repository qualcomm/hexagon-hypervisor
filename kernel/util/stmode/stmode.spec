:mod:`stmode` -- Single-thread mode assistance
================================================

.. module:: stmode

Single-thread mode temporarily disables global interrupts while the kernel
checks whether only one non-waiting hardware thread remains active.  The BKL
serializes the entry check.


H2K_stmode_begin
----------------

.. c:function:: s32_t H2K_stmode_begin(void) IN_SECTION(".text.misc.stmode")

	:returns: 0 when single-thread mode is entered, otherwise ``-1``

Acquires the BKL and clears global interrupt enable.  It examines ``MODECTL``
and removes waiting threads from the active-thread mask.  If no more than one
thread remains active, returns 0 after releasing the BKL, leaving global
interrupts disabled.  Otherwise restores global interrupt enable, releases the
BKL, and returns ``-1``.


H2K_stmode_end
--------------

.. c:function:: void H2K_stmode_end(void) IN_SECTION(".text.misc.stmode")

Restores global interrupt enable by setting the global-interrupt bit in
``SYSCFG``.
