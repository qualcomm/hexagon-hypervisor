
:mod:`subsystem` -- Coprocessor subsystem power-up
====================================================

.. module:: subsystem

H2K_hvx_init
------------

.. c:function:: void H2K_hvx_init(u32_t devpage_offset)

	:param devpage_offset: Offset of the subsystem device page

Description
~~~~~~~~~~~

Powers up the HVX coprocessor.  Available only when ``HAVE_EXTENSIONS`` is
enabled.

Functionality
~~~~~~~~~~~~~

If HVX was not detected during boot, the function returns immediately.
Otherwise, it records the addresses of the HVX clock, reset, and power-control
registers.  On architecture V65 and later, it also records the addresses of
the HVX BHS and coprocessor-memory registers.  Finally, it sets the HVX state
to off.


H2K_hmx_init
------------

.. c:function:: void H2K_hmx_init(u32_t devpage_offset)

	:param devpage_offset: Offset of the subsystem device page

Description
~~~~~~~~~~~

Powers up the HMX coprocessor.  Available only when ``HAVE_EXTENSIONS`` is
enabled.

Functionality
~~~~~~~~~~~~~

If HMX was not detected during boot, the function returns immediately.
Otherwise, on architecture V68 and later, it records the addresses of the HMX
resource-sequencer registers and sets the HMX state to off.


H2K_hlx_init
------------

.. c:function:: void H2K_hlx_init(u32_t devpage_offset)

	:param devpage_offset: Offset of the subsystem device page

TODO: implement HLX power-up (if needed).
