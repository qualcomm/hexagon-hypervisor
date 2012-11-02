:mod:`vmcache` -- Cache Maintenance Requests
============================================

.. module:: vmcache


H2K_vmtrap_cachectl
-------------------

.. cfunction:: void H2K_vmtrap_cachectl(H2K_thread_context *me)

	:param me: Pointer to the current thread context

Description
~~~~~~~~~~~

This function handles the "vmcache" virtual instruction, to 
clean and/or invalidate various caches.

Functionality
~~~~~~~~~~~~~

The appropriate caches are cleaned and/or invalidated, per request.
In no case is data lost; the dcinva instruction is not used.





