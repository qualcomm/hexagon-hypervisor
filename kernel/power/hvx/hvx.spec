
:mod:`hvx` -- HVX Power Control
===============================

.. module:: hvx

H2K_hvx_poweron
---------------

.. cfunction:: void H2K_hvx_poweron(void)

Description
~~~~~~~~~~~

Power HVX on.

Functionality
~~~~~~~~~~~~~

Check and set HVX power state bit atomically.  If power was off, execute the power-on sequence.


H2K_hvx_poweroff
----------------

.. cfunction:: void H2K_hvx_poweroff(void)

Description
~~~~~~~~~~~

Power HVX off.

Functionality
~~~~~~~~~~~~~

Check and set HVX power state bit atomically.  If power was on, execute the power-off sequence.
