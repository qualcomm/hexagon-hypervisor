
:mod:`tid` -- managing the software thread id
==============================================


H2K_tid_set
-----------

.. cfunction:: void H2K_tid_set(u32_t tid, H2K_thread_context *me)

Description
~~~~~~~~~~~

Set the TID value

Input
~~~~~

Argument 0: the new TID value for the current thread
Argument 1: Pointer to the context for the current thread

Output
~~~~~~

Functionality
~~~~~~~~~~~~~

We set the TID value in ``me->tid``, and also put it in the TID register.


H2K_tid_get
-----------

.. cfunction:: u32_t H2K_tid_get(H2K_thread_context *me)

Description
~~~~~~~~~~~

Get the TID value

Input
~~~~~

Argument 0: pointer to the context for the current thread

Output
~~~~~~

Returns the chosen TID

Functionality
~~~~~~~~~~~~~

We read the value ``me->tid``, and return the resulting value.



Testing
-------

Samples
~~~~~~~

* Value in me->tid
* Value in the TID register


Important Cases
~~~~~~~~~~~~~~~

* Setting TID
* Getting TID value


Harness
~~~~~~~

Fairly standard.  Test various values of TID.  

For setting TID, check that both the register and context are set correctly.

For getting TID, check that the value in the struct is returned.

