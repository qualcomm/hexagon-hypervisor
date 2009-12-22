
:mod:`intconfig` -- configuration for interrupt handling
=========================================================

.. module:: intconfig

H2K_intconfig_init
------------------

.. cfunction:: void H2K_intconfig_init()

Description
~~~~~~~~~~~

H2K_intconfig_init initializes the interrupt configuration data and fast interrupt contexts

Input
~~~~~

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

All handler pointers are initialized to NULL.  The fastint mask is initialized 
to zeros.  The handler for reschedule is initialized to H2K_resched.

Next, we initialize all fastint contexts to zeros.  We then update the hthread
fields to the correct hardware thread that will be using the fastint context, 
and the trapmask to exclude any trap may not be called by the fast interrupt
handler.



H2K_register_fastint
--------------------

.. cfunction:: void H2K_register_fastint(u32_t whatint, void (*fastint_handler)(u32_t x), H2K_thread_context *me)

Description
~~~~~~~~~~~

Modifies the interrupt configuration data to register a new fast interrupt handler.

Input
~~~~~

Argument 0: which interrupt to register
Argument 1: Address of the fast interrupt handler
Argument 2: Context of the calling thread

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

The fastint_handler is placed in the array, and H2K_fastint is placed in the other array




Testing
-------

For H2K_intconfig_init, we check to make sure that the array has been
initialized after the call.

Samples
~~~~~~~

* Input: fastint_handler
* Input: whatint
* Output: H2K_fastint_funcptrs[whatint]
* Output: H2K_inthandlers[whatint]
* I/O: H2K_fastint_mask: bit whatint set
* Output: IAD bit for interrupt whatint cleared

