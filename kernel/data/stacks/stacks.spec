
ASM_REF_CODE(Stack labels appear after data, easier to represent in asm)

:mod:`stacks` -- data for kernel stacks
========================================

.. module:: stacks

H2K_stacks
----------

Contains kernel stacks.

Description
~~~~~~~~~~~

Kernel stacks size is defined by constants in max.h.  We create one kernel
stack per hardware thread.  The kernel stack for a hardware thread is obtained
by multiplying the hardware thread number by KERNEL_STACK_SIZE, and adding it
to the kernel stack address.  Because stacks grow towards lower addresses, this
means that the label for the kernel stack address should point to the end of
the data area for hardware thread zero.

