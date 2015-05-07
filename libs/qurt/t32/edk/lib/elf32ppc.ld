/* LD linker script for Lauterbach TRACE32 Extensions (use with elf2t32) */
OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc", "elf32-powerpc-littleendian")
OUTPUT_ARCH(powerpc)
ENTRY(_start)
SECTIONS
{
  /* Read-only sections, merged into text segment: */
  . = 0x40000 + SIZEOF_HEADERS;
  .interp	: { *(.interp)		}
  .hash		: { *(.hash)		}
  .dynsym	: { *(.dynsym)		}
  .dynstr	: { *(.dynstr)		}
  .rela.text	: { *(.rela.text) 	}
  .rela.data	: { *(.rela.data) 	}
  .rela.rodata	: { *(.rela.rodata) 	}
  .rela.got	: { *(.rela.got)	}
  .rela.got1	: { *(.rela.got1)	}
  .rela.got2	: { *(.rela.got2)	}
  .rela.ctors	: { *(.rela.ctors)	}
  .rela.dtors	: { *(.rela.dtors)	}
  .rela.init	: { *(.rela.init)	}
  .rela.fini	: { *(.rela.fini)	}
  .rela.bss	: { *(.rela.bss)	}
  .rela.plt	: { *(.rela.plt)	}
  .rela.sdata	: { *(.rela.sdata2)	}
  .rela.sbss	: { *(.rela.sbss2)	}
  .rela.sdata2	: { *(.rela.sdata2)	}
  .rela.sbss2	: { *(.rela.sbss2)	}
  .init	0x00000	: { *(.init)		} =0
  .plt		: { *(.plt)		}
  /* text starts at 0x00000 - if you change it here, also change elf2t32.c */ 
  .text	0x00000	: { *(.text)
  		    *(.gnu.warning)	/* handled specially by elf32.em */
		    			} =0
  PROVIDE (etext = .);
  .fini		: { *(.fini)		} =0
  .rodata	: { *(.rodata)		}
  .rodata1	: { *(.rodata1)		}
  etext = .;
  .sdata2	: { *(.sdata2)		}  
  PROVIDE (__SDATA2_START__ = .);	/* bug? this is not sdata2_start! */
  .sbss2	: { *(.sbss2)		} 
  PROVIDE (__SBSS2_END__ = .);
  . = ALIGN(8) + 0x40000;		/* alignment */
  /* data starts at 0x20000 - if you change it here, also change elf2t32.c */ 
  .data 0x20000	: { *(.data)
  		    CONSTRUCTORS	}
  .data1	: { *(.data1) 		}
  .got1		: { *(.got1)		}
  .dynamic	: { *(.dynamic)		}
  /* The current compiler no longer needs this, but keep it around for 2.7.2  */
  PROVIDE (_GOT2_START_ = .);
  .got2		:  { *(.got2)		}
  PROVIDE (__CTOR_LIST__ = .);
  .ctors	: { *(.ctors) 		}
  PROVIDE (__CTOR_END__ = .);
  PROVIDE (__DTOR_LIST__ = .);
  .dtors	: { *(.dtors)		}
  PROVIDE (__DTOR_END__ = .);
  PROVIDE (_FIXUP_START_ = .);
  .fixup	: { *(.fixup)		}
  PROVIDE (_FIXUP_END_ = .);
  PROVIDE (_GOT2_END_ = .);
  PROVIDE (_GOT_START_ = .);
  PROVIDE (_GLOBAL_OFFSET_TABLE_ = .);
  .got		: { *(.got)		}
  .got.plt	: { *(.got.plt)		}
  .sbss2	: { *(.sbss2)		}
  PROVIDE (_GOT_END_ = .);
  /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
  PROVIDE (__SDATA_START__ = .);
  .sdata	: { *(.sdata)		}
  _edata  =  .;
  PROVIDE (edata = .);
  PROVIDE (__sbss_start = .);
  .sbss		: { *(.sbss)
		    *(.scommon)		}
  PROVIDE (__sbss_end = .);
  PROVIDE (__SBSS_END__ = .);
  PROVIDE (__bss_start = .);
  .bss		: { *(.dynbss)
		    *(.bss)
		    *(COMMON)		}
  _end = . ;
  PROVIDE (end = .);
  /* Needed for ELF backends not yet converted to the new style linker: */
  .stab           0 : { *(.stab)		}
  .stabstr        0 : { *(.stabstr)		}
  /* DWARF debug sections: */
  .debug          0 : { *(.debug)		}
  .debug_srcinfo  0 : { *(.debug_srcinfo)	}
  .debug_aranges  0 : { *(.debug_aranges)	}
  .debug_pubnames 0 : { *(.debug_pubnames)	}
  .debug_sfnames  0 : { *(.debug_sfnames)	}
  .line           0 : { *(.line)		}
}/*SECTIONS*/
