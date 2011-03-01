:mod:`vmipi` -- Virtual Machine Interupt
=============================================

.. module:: vmipi


H2K_vm_do_ipi
-------------

.. cfunction:: void H2K_vm_do_ipi(H2K_thread_context *);

Description
~~~~~~~~~~~
clear bit in globals ipimask
if ipi mask non-zero, setimask for ipi interrupt with ~ipimask and reraise


H2K_vm_send_ipi
---------------

.. cfunction:: void H2K_vm_send_ipi(H2K_thread_context *);

Description
~~~~~~~~~~~
set bit in globals ipimask
setimask for ipi interrupt with ~ipimask and raise


