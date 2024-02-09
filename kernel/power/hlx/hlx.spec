//TODO:REDO for HLX
:mod:`hlx` -- HLX Power Control
===============================

.. module:: hlx

H2K_hlx_poweron
---------------

.. c:function:: void H2K_hlx_poweron(void)

Description
~~~~~~~~~~~

Power HLX on.

Functionality
~~~~~~~~~~~~~

Check and set HLX power state bit atomically.  If power was off, execute the power-on sequence.


H2K_hlx_poweroff
----------------

.. c:function:: void H2K_hlx_poweroff(void)

Description
~~~~~~~~~~~

Power HLX off.

Functionality
~~~~~~~~~~~~~

Check and set HLX power state bit atomically.  If power was on, execute the power-off sequence.
