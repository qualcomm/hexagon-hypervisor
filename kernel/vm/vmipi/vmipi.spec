:mod:`vmipi` -- Virtual Machine Interupt
=============================================

.. module:: vmipi


H2K_vm_do_ipi
-------------

.. c:function:: void H2K_vm_do_ipi(H2K_thread_context *);

Description
~~~~~~~~~~~
clear bit in globals ipimask
if ipi mask non-zero, setimask for ipi interrupt with ~ipimask and reraise


H2K_vm_ipi_send
---------------

.. c:function:: void H2K_vm_ipi_send(H2K_thread_context *);

Description
~~~~~~~~~~~
set bit in globals ipimask
setimask for ipi interrupt with ~ipimask and raise


H2K_vm_ipi_send_withlock
------------------------

.. c:function:: void H2K_vm_ipi_send_withlock(H2K_thread_context *);

Description
~~~~~~~~~~~

Same as :c:func:`H2K_vm_ipi_send()`, but while holding the BKL.  BKL is still held on return.


