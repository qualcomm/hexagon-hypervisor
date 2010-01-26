
:mod:`bootmap` -- Memory Map at Bootup
======================================

.. module:: bootmap

This module contains the memory map during the boot process.

bootmap.def
-----------

This file contains definitions for the boot memory map.

The file supports "//" as a comment indicator.

Each memory map entry should have the form: MEMORY_MAP( <args> ) on a single line.

Arguments:

0. Global: is this translation shared across all address spaces?  Kernel translations must be.
1. ASID: Address Space ID for this translation.
2. Virtual Page Number.  Virtual address for the start of the page, shifted right by 12 bits.
3. Permissions.  Should be one of:
	* 0. No user permissions
	* "R". Read permissions only.
	* "W". Write permissions only.
	* "X". Execute permissions only.
	* "RW". Read and Write Permissions.
	* "RX". Read and Execute Permissions.
	* "WX". Write and Execute Permissions.
	* "RWX". Read, Write, and Execute Permissions.
4. Cacheability.  Should be one of:
	* "L1WB_L2UC": L1 WriteBack, Non-L2 Cacheable
	* ...
5. PGSIZE.  Page Size.  SIZE_XX, where XX is a valid page size from "4K" to "16M".
6. Partition.  Must be "MAIN" or "AUX"
7. PPN.  Physical address, shifted right by 12 bits.


H2K_bootmap
-----------

Description
~~~~~~~~~~~

This is an array that contains the translations specified in bootmap.def, 
terminated by an invalid translation.  

H2K_bootmap_ptr
---------------

Description
~~~~~~~~~~~

This points to the bootup memory map.  The boot process loads through this
pointer to add potential configurability.

