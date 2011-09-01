
:mod:`tlbfmt` -- Hardware TLB Entry Format Services
===================================================

.. module:: tlbfill

H2K_mem_tlbfmt_t
----------------

.. ctype:: H2K_mem_tlbfmt_t

        Pagetable format (<= V3)
	
	.. cmember:: u64_t raw

	          Provides raw access to the entire structure.
		  Include "low" and "high"

                  .. cmember:: u32_t low

	                    Provides raw access to the "low" portion of the table.

		            .. cmember:: u32_t ppn
			    
			              Physical Page Number to which the corresponding Virtual Page maps.
				      20 bits;

			    .. cmember:: u32_t size

			              Page size: 4K to 16M (see bootmap.ref.c)
				      4 bits; Only the 3 least significant are used.
	
			    .. cmember:: u32_t part

			              Partition bit.  Turn on cache partitioning.
				      2 bits; Only the least significant bit is used.

			    .. cmember:: u32_t ccc

			              Cacheability Attributes. L1WB_L2UC ... (see bootmap.ref.c)
				      3 bits;

			    .. cmember:: u32_t xwr

			              User-mode permissions. { eXecute, Write, Read }
				      3 bits;

                  .. cmember:: u32_t high

	                    Provides raw access to the "high" portion of the table.

		            .. cmember:: u32_t vpn
			    
			              Virtual Page Number.
				      20 bits;

			    .. cmember:: u32_t asid

			              Address Space Identifier
				      5 bits;
	
			    .. cmember:: u32_t guestonly
			              
			              Guest permissions vs user permissions.
				      1 bits

			    .. cmember:: u32_t unused

			              Unused
				      2 bits;

			    .. cmember:: u32_t global

			              Global page, ignore ASID if this is true.
				      1 bits;
			      
			    .. cmember:: u32_t valid

			              Valid bit for page.
				      1 bit;

			    .. cmember:: u32_t unused2

			              Unused
				      2 bits;

        Pagetable format (>= V4)
	
	.. cmember:: u64_t raw

	          Provides raw access to the entire structure.
		  Include "low" and "high"

                  .. cmember:: u32_t low

	                    Provides raw access to the "low" portion of the table.

		            .. cmember:: u32_t ppd
			    
			              Physical Page Description.  Physical Address along with size information
				      24 bits;

			    .. cmember:: u32_t cccc

			              Cacheability Attributes. L1WB_L2UC ... (see bootmap.ref.c)
				      4 bits;
	
			    .. cmember:: u32_t xwru

			              Mode permissions. { eXecute, Write, Read, User }
				      4 bits;

                  .. cmember:: u32_t high

	                    Provides raw access to the "high" portion of the table.

		            .. cmember:: u32_t vpn
			    
			              Virtual Page Number.
				      20 bits;

			    .. cmember:: u32_t asid

			              Address Space Identifier
				      7 bits;
	
			    .. cmember:: u32_t unused

			              Unused
				      3 bits;

			    .. cmember:: u32_t global

			              Global page, ignore ASID if this is true.
				      1 bit;
			      
			    .. cmember:: u32_t valid

			              Valid bit for page.
				      1 bit;

	

H2K_mem_tlbfmt_from_linear
--------------------------

.. cfunction:: H2K_mem_tlbfmt_t H2K_mem_tlbfmt_from_linear(H2K_linear_t linear, u32_t asid)

Description
~~~~~~~~~~~

This function converts a linear translation table entry and ASID to the native
Hardware TLB entry format.

H2K_mem_tlbfmt_from_table
-------------------------

.. cfunction:: H2K_mem_tlbfmt_t H2K_mem_tlbfmt_from_table(u32_t va, u32_t asid, pte_t pte)

Description
~~~~~~~~~~~

This function converts a page table entry, VA, and ASID to the native Hardware
TLB entry format.


H2K_mem_tlbfmt_get_size
-----------------------

.. cfunction:: u32_t H2K_mem_tlbfmt_get_size(H2K_mem_tlbfmt_t entry)

Description
~~~~~~~~~~~

Returns the page size of the entry:

	0. 4KB Translation
	1. 16KB Translation
	2. 64KB Translation
	3. 256KB Translation
	4. 1MB Translation
	5. 4MB Translation
	6. 16MB Translation




H2K_mem_tlbfmt_get_perms
------------------------

.. cfunction:: u32_t H2K_mem_tlbfmt_get_perms(H2K_mem_tlbfmt_t entry)

Description
~~~~~~~~~~~

Returns the OR of the permissions of the entry:

	1. User permission
	2. Read permission
	4. Write permission
	8. Execute permission


H2K_mem_tlbfmt_get_basepa
-------------------------

.. cfunction:: pa_t H2K_mem_tlbfmt_get_basepa(H2K_mem_tlbfmt_t entry)

Description
~~~~~~~~~~~

Returns the base physical address of the translation.  Note that the base physical 
address may not be aligned to the size; the caller must mask off the appropriate bits.


