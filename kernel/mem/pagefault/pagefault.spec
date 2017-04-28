
:mod:`pagefault` -- Handle a Missing Translation
================================================

.. module:: pagefault


H2K_mem_pagefault
-----------------

.. c:function:: void H2K_mem_pagefault(u32_t va, H2K_thread_context *me)

Description
~~~~~~~~~~~

This code reconfigures the thread to return to the event handler, indicating
a page fault, if an event handler has been registered.  If no event handler
has been registered, the thread is terminated.

Functionality
~~~~~~~~~~~~~

The Guest BADVA register is set to va.  The Guest Status Register is set to 
indicate a page fault, ...

These values are all changed in the thread context pointed to by "me".  
The code on the return path will load the values from the context and 
end up at the correct location with the correct values in place.

