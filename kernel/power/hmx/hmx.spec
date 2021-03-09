
:mod:`hmx` -- HMX Power Control
===============================

.. module:: hmx

H2K_hmx_poweron
---------------

.. c:function:: void H2K_hmx_poweron(void)

Description
~~~~~~~~~~~

Power HMX on.

Functionality
~~~~~~~~~~~~~

Check and set HMX power state bit atomically.  If power was off, execute the power-on sequence.


H2K_hmx_poweroff
----------------

.. c:function:: void H2K_hmx_poweroff(void)

Description
~~~~~~~~~~~

Power HMX off.

Functionality
~~~~~~~~~~~~~

Check and set HMX power state bit atomically.  If power was on, execute the power-off sequence.
