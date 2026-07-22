/*
 * vapa_kernel_picolibc.x - monitor-space linker script for picolibc test builds.
 *
 * Used by STANDALONE and non-STANDALONE non-BOOT tests when PICOLIBC=1.
 * These tests run in monitor space (linked at H2K_LINK_ADDR = 0xff000000).
 *
 * Derived from vapa_bootvm_picolibc.x with the following changes:
 *   - No .entry section (bootvm-specific; not needed for tests)
 *   - .start placement controlled by -Wl,--section-start=.start=H2K_LINK_ADDR
 *     passed from Makefile.inc.test (same as the non-picolibc path)
 *   - __data_source/__data_start/__data_size: for monitor-space tests .data is
 *     already in RAM so source == start and size = 0 (no ROM-to-RAM copy needed)
 *
 * Picolibc-required symbols provided:
 *   __stack          - top-of-stack; set as r29 by our custom _start
 *   __bss_start/__bss_size - BSS region for zeroing in _start
 *   __tdata_start/__tdata_end/__tdata_size - TLS initialised-data bookkeeping
 *   __tbss_start/__tbss_end/__tbss_size/__tbss_offset - TLS BSS bookkeeping
 *   __tls_base       - per-thread TLS block for the main thread
 *   __heap_start/end - used by picolibc malloc sbrk
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

OUTPUT_FORMAT("elf32-littlehexagon", "elf32-littlehexagon",
	      "elf32-littlehexagon")
ENTRY(_start)
SECTIONS
{
  PROVIDE (__executable_start = SEGMENT_START("text-segment", 0)); . = SEGMENT_START("text-segment", 0);

  .interp         : { *(.interp) }
  .note.gnu.build-id : { *(.note.gnu.build-id) }
  .hash           : { *(.hash) }
  .gnu.hash       : { *(.gnu.hash) }
  .dynsym         : { *(.dynsym) }
  .dynstr         : { *(.dynstr) }
  .gnu.version    : { *(.gnu.version) }
  .gnu.version_d  : { *(.gnu.version_d) }
  .gnu.version_r  : { *(.gnu.version_r) }
  .rela.init      : { *(.rela.init) }
  .rela.text      : { *(.rela.text .rela.text.* .rela.gnu.linkonce.t.*) }
  .rela.fini      : { *(.rela.fini) }
  .rela.rodata    : { *(.rela.rodata .rela.rodata.* .rela.gnu.linkonce.r.*) }
  .rela.data.rel.ro : { *(.rela.data.rel.ro* .rela.gnu.linkonce.d.rel.ro.*) }
  .rela.data      : { *(.rela.data .rela.data.* .rela.gnu.linkonce.d.*) }
  .rela.tdata     : { *(.rela.tdata .rela.tdata.* .rela.gnu.linkonce.td.*) }
  .rela.tbss      : { *(.rela.tbss .rela.tbss.* .rela.gnu.linkonce.tb.*) }
  .rela.ctors     : { *(.rela.ctors) }
  .rela.dtors     : { *(.rela.dtors) }
  .rela.got       : { *(.rela.got) }
  .rela.sdata     : { *(.rela.sdata .rela.lit[a48] .rela.sdata.* .rela.lit[a48].* .rela.gnu.linkonce.s.* .rela.gnu.linkonce.l[a48].*) }
  .rela.sbss      : { *(.rela.sbss .rela.sbss.* .rela.gnu.linkonce.sb.*) }
  .rela.sdata2    : { *(.rela.sdata2 .rela.sdata2.* .rela.gnu.linkonce.s2.*) }
  .rela.sbss2     : { *(.rela.sbss2 .rela.sbss2.* .rela.gnu.linkonce.sb2.*) }
  .rela.bss       : { *(.rela.bss .rela.bss.* .rela.gnu.linkonce.b.*) }
  .rela.iplt      :
  {
    PROVIDE_HIDDEN (__rela_iplt_start = .);
    *(.rela.iplt)
    PROVIDE_HIDDEN (__rela_iplt_end = .);
  }
  .rela.plt       : { *(.rela.plt) }

  /* Code - .start placed at H2K_LINK_ADDR by -Wl,--section-start */
  . = ALIGN (DEFINED (TEXTALIGN) ? (TEXTALIGN * 1K) : CONSTANT (MAXPAGESIZE));
  .start :
  {
    KEEP (*(.entry))
    KEEP (*(.start))
    /* picolibc places _start in .text.init.enter; keep it right after .start */
    KEEP (*(.text.init.enter))
  } =0x00c0007f
  .init  : { KEEP (*(.init)) } =0x00c0007f
  .plt   : { *(.plt) }
  .iplt  : { *(.iplt) }
  . = ALIGN (. + CONSTANT (COMMONPAGESIZE), CONSTANT (COMMONPAGESIZE));
  .text  :
  {
    *(.text.unlikely .text.*_unlikely)
    *(.text.hot .text.hot.* .gnu.linkonce.t.hot.*)
    *(.text .stub .text.* .gnu.linkonce.t.*)
    *(.gnu.warning)
    . = ALIGN(16);
    __start___lcxx_override = .;
    *(__lcxx_override)
    __stop___lcxx_override = .;
  } =0x00c0007f
  .fini  : { KEEP (*(.fini)) } =0x00c0007f
  PROVIDE (__etext = .);
  PROVIDE (_etext = .);
  PROVIDE (etext = .);

  /* picolibc constructor/destructor arrays */
  . = ALIGN(8);
  PROVIDE_HIDDEN (__bothinit_array_start = .);
  .preinit_array :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  }
  .ctors :
  {
    PROVIDE_HIDDEN (__ctors_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.ctors.*)))
    KEEP (*(.ctors))
    PROVIDE_HIDDEN (__ctors_end = .);
  }
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.init_array.*)))
    KEEP (*(.init_array))
    PROVIDE_HIDDEN (__init_array_end = .);
  }
  PROVIDE_HIDDEN (__bothinit_array_end = .);
  .dtors :
  {
    PROVIDE_HIDDEN (__dtors_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.dtors.*)))
    KEEP (*(.dtors))
    PROVIDE_HIDDEN (__dtors_end = .);
  }
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT_BY_INIT_PRIORITY(.fini_array.*)))
    KEEP (*(.fini_array))
    PROVIDE_HIDDEN (__fini_array_end = .);
  }

  /* Read-only data */
  . = ALIGN (. + CONSTANT (COMMONPAGESIZE), CONSTANT (COMMONPAGESIZE));
  .rodata :
  {
    *(.rodata.hot .rodata.hot.* .gnu.linkonce.r.hot.*)
    *(.rodata .rodata.* .gnu.linkonce.r.*)
  }
  .rodata1        : { *(.rodata1) }
  .eh_frame_hdr   :
  {
    __eh_frame_hdr_start = .;
    KEEP (*(.eh_frame_hdr))
    __eh_frame_hdr_end = .;
  }
  .eh_frame       : ONLY_IF_RO { KEEP (*(.eh_frame)) }
  .gcc_except_table : ONLY_IF_RO { *(.gcc_except_table .gcc_except_table.*) }

  /* Data segment */
  . = ALIGN (CONSTANT (MAXPAGESIZE)) - ((CONSTANT (MAXPAGESIZE) - .) & (CONSTANT (MAXPAGESIZE) - 1));
  . = DATA_SEGMENT_ALIGN (CONSTANT (MAXPAGESIZE), CONSTANT (COMMONPAGESIZE));
  . = ALIGN (DEFINED (DATAALIGN) ? (DATAALIGN * 1K) : CONSTANT (MAXPAGESIZE));

  .eh_frame       : ONLY_IF_RW { KEEP (*(.eh_frame)) }
  .gcc_except_table : ONLY_IF_RW { *(.gcc_except_table .gcc_except_table.*) }

  /* ------------------------------------------------------------------ */
  /* TLS sections - required by picolibc _set_tls                        */
  /* ------------------------------------------------------------------ */
  . = ALIGN(8);
  .tdata :
  {
    __tdata_source = .;
    __tdata_start  = __tdata_source;
    *(.tdata .tdata.* .gnu.linkonce.td.*)
  }
  __tdata_end  = .;
  __tdata_size = __tdata_end - __tdata_source;

  .tbss :
  {
    __tbss_start = .;
    *(.tbss .tbss.* .gnu.linkonce.tb.*) *(.tcommon)
    . = ALIGN(8);
    __tbss_end = .;
  }
  __tbss_size   = __tbss_end - __tbss_start;
  __tbss_offset = __tbss_start - __tdata_start;

  /* Per-thread TLS space for the main thread */
  . = ALIGN(4K);
  .tls_space (NOLOAD) :
  {
    __tls_base       = .;
    __tls_space_size = __tbss_end - __tdata_start;
    . += __tls_space_size;
  }
  /* ------------------------------------------------------------------ */

  . = ALIGN(4K);
  /* For monitor-space tests .data is already in RAM (no ROM-to-RAM copy).
   * Set source == start and size = 0 to satisfy any crt0 that uses these. */
  __data_source = .;
  __data_start  = .;
  .data :
  {
    *(.data.hot .data.hot.* .gnu.linkonce.d.hot.*)
    *(.data .data.* .gnu.linkonce.d.*)
    __start___llvm_prf_cnts = .;
    KEEP(*(__llvm_prf_cnts))
    __stop___llvm_prf_cnts = .;
    __start___llvm_prf_data = .;
    KEEP(*(__llvm_prf_data))
    __stop___llvm_prf_data = .;
    SORT(CONSTRUCTORS)
  }
  __data_size = . - __data_start;
  .data1 : { *(.data1) }
  _edata = .; PROVIDE (edata = .);

  . = ALIGN (64);
  .sdata :
  {
    PROVIDE (_SDA_BASE_ = .);
    *(.sdata.1 .sdata.1.* .gnu.linkonce.s.1.*)
    *(.sbss.1  .sbss.1.*  .gnu.linkonce.sb.1.*)
    *(.scommon.1 .scommon.1.*)
    *(.sdata.2 .sdata.2.* .gnu.linkonce.s.2.*)
    *(.sbss.2  .sbss.2.*  .gnu.linkonce.sb.2.*)
    *(.scommon.2 .scommon.2.*)
    *(.sdata.4 .sdata.4.* .gnu.linkonce.s.4.*)
    *(.sbss.4  .sbss.4.*  .gnu.linkonce.sb.4.*)
    *(.scommon.4 .scommon.4.*)
    *(.lit[a4] .lit[a4].* .gnu.linkonce.l[a4].*)
    *(.sdata.8 .sdata.8.* .gnu.linkonce.s.8.*)
    *(.sbss.8  .sbss.8.*  .gnu.linkonce.sb.8.*)
    *(.scommon.8 .scommon.8.*)
    *(.lit8 .lit8.* .gnu.linkonce.l8.*)
    *(.sdata.hot .sdata.hot.* .gnu.linkonce.s.hot.*)
    *(.sdata .sdata.* .gnu.linkonce.s.*)
  }
  .got : { *(.got) *(.igot) }
  . = DATA_SEGMENT_RELRO_END (16, .);
  .got.plt : { *(.got.plt) *(.igot.plt) }

  .sbss :
  {
    PROVIDE (__sbss_start = .);
    PROVIDE (___sbss_start = .);
    *(.dynsbss)
    *(.sbss.hot .sbss.hot.* .gnu.linkonce.sb.hot.*)
    *(.sbss .sbss.* .gnu.linkonce.sb.*)
    *(.scommon .scommon.*)
    . = ALIGN (. != 0 ? 64 : 1);
    PROVIDE (__sbss_end = .);
    PROVIDE (___sbss_end = .);
  }

  . = ALIGN (64);
  /* ------------------------------------------------------------------ */
  /* BSS - zeroed by our custom _start via memset                        */
  /* ------------------------------------------------------------------ */
  __bss_start = .;
  .bss (NOLOAD) :
  {
    *(.dynbss)
    *(.bss.hot .bss.hot.* .gnu.linkonce.b.hot.*)
    *(.bss .bss.* .gnu.linkonce.b.*)
    *(COMMON)
    . = ALIGN (. != 0 ? 64 : 1);
  }
  __bss_end  = .;
  __bss_size = __bss_end - __bss_start;
  /* ------------------------------------------------------------------ */

  . = ALIGN (64);
  _end = .;
  PROVIDE (end = .);

  /* ------------------------------------------------------------------ */
  /* Heap - picolibc sbrk uses __heap_start / __heap_end                 */
  /* Falls back to 0x10000 if HEAP_SIZE defsym not provided              */
  /* ------------------------------------------------------------------ */
  __heap_start = .;
  .heap (NOLOAD) :
  {
    . += (DEFINED(HEAP_SIZE) ? HEAP_SIZE : 0x10000);
  }
  __heap_end = .;
  /* ------------------------------------------------------------------ */

  /* ------------------------------------------------------------------ */
  /* Stack - our _start sets r29 = __stack (top of stack)               */
  /* Falls back to 0x1000 if STACK_SIZE defsym not provided             */
  /* ------------------------------------------------------------------ */
  .stack (NOLOAD) :
  {
    . += (DEFINED(STACK_SIZE) ? STACK_SIZE : 0x1000);
  }
  __stack = .;
  STACK_SIZE = DEFINED(STACK_SIZE) ? STACK_SIZE : 0x1000;
  /* ------------------------------------------------------------------ */

  . = DATA_SEGMENT_END (.);

  .hexagon.attributes 0 : { *(.hexagon.attributes) }
  .comment       0 : { *(.comment) }
  .debug          0 : { *(.debug) }
  .line           0 : { *(.line) }
  .debug_aranges  0 : { *(.debug_aranges) }
  .debug_pubnames 0 : { *(.debug_pubnames) }
  .debug_info     0 : { *(.debug_info .gnu.linkonce.wi.*) }
  .debug_abbrev   0 : { *(.debug_abbrev) }
  .debug_line     0 : { *(.debug_line) }
  .debug_frame    0 : { *(.debug_frame) }
  .debug_str      0 : { *(.debug_str) }
  .debug_loc      0 : { *(.debug_loc) }
  .debug_pubtypes 0 : { *(.debug_pubtypes) }
  .debug_ranges   0 : { *(.debug_ranges) }
  /DISCARD/ : { *(.note.GNU-stack) *(.gnu_debuglink) *(.gnu.lto_*) *(.note.gnu.build-id) }
}
