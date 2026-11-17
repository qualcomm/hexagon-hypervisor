:mod:`max` -- Shared compile-time limits and hardware definitions
================================================================

.. module:: max

``max.h`` is a definitions-only header shared by kernel C and assembly modules.
It contains no callable interface.  The definitions describe supported core
families, kernel capacities, fixed virtual mappings, interrupt topology,
register fields, boot state, and hardware configuration-table layouts.

``ARCHV`` selects the implementation family compiled into the kernel.  Runtime
core revision values are represented separately by the ``CORE_V*`` and
``CORE_V6_*`` constants.  Thus a runtime revision may use an older compatible
``ARCHV`` implementation family.

Some inputs used here are supplied by common headers or the build:

* ``MAX_HTHREADS`` comes from ``h2_common_defs.h`` unless overridden.  Its
  default is 4 through ARCHV 65, 8 for ARCHV 68 and 73, and 16 from ARCHV 81.
* ``PAGE_BITS`` and the ``SIZE_*`` page-size encodings come from the common
  physical-map interface.
* ``HAVE_EXTENSIONS`` is enabled for ARCHV >= 60 except for tiny-core builds.
* ``H2K_KERNEL_PGSIZE`` selects the kernel page-size encoding.


Core revision and capacity
--------------------------

.. list-table:: General limits and revision fields
   :header-rows: 1

   * - Definition
     - Value or meaning
   * - ``MAX_CORES``
     - 8 processor cores
   * - ``CORE_REV_ARCH_MASK``
     - ``0xff``; isolates the architecture field of the core revision
   * - ``CORE_REV_L2_CHUNK_SWITCH``
     - ``0x8``; revision point at which legacy L2 sizing uses larger chunks
   * - ``L2_TAG_CHUNK``
     - ``0x8000`` bytes (32 KiB)
   * - ``L2_CHUNK``
     - ``0x20000`` bytes (128 KiB)
   * - ``L2_BIG_CHUNK``
     - ``0x40000`` bytes (256 KiB)
   * - ``MAX_HTHREADS``
     - Maximum hardware threads supported by the compiled implementation;
       imported from ``h2_common_defs.h``
   * - ``MAX_HTHREADS_MASK``
     - Mask containing one bit for every supported hardware thread

The runtime architecture identifiers are equal to their hexadecimal revision
numbers:

.. list-table:: Runtime architecture identifiers
   :header-rows: 1

   * - Definition
     - Value
   * - ``CORE_V4``, ``CORE_V5``
     - ``0x04``, ``0x05``
   * - ``CORE_V60``, ``CORE_V61``, ``CORE_V62``
     - ``0x60``, ``0x61``, ``0x62``
   * - ``CORE_V65``, ``CORE_V66``, ``CORE_V67``
     - ``0x65``, ``0x66``, ``0x67``
   * - ``CORE_V68``, ``CORE_V69``
     - ``0x68``, ``0x69``
   * - ``CORE_V70``, ``CORE_V71``, ``CORE_V72``, ``CORE_V73``
     - ``0x70``, ``0x71``, ``0x72``, ``0x73``
   * - ``CORE_V75``, ``CORE_V77``, ``CORE_V79``
     - ``0x75``, ``0x77``, ``0x79``
   * - ``CORE_V81``, ``CORE_V85``, ``CORE_V87``, ``CORE_V89``, ``CORE_V91``
     - ``0x81``, ``0x85``, ``0x87``, ``0x89``, ``0x91``

The V6 microarchitecture identifiers are ``CORE_V6_A`` = ``0x0``,
``CORE_V6_B`` = ``0x1``, ``CORE_V6_S`` = ``0x2``, ``CORE_V6_C`` = ``0x3``,
``CORE_V6_G`` = ``0x4``, ``CORE_V6_U`` = ``0x5``, ``CORE_V6_H`` = ``0x6``,
``CORE_V6_D`` and ``CORE_V6_J`` = ``0x7``, ``CORE_V6_E`` and ``CORE_V6_P`` =
``0x8``, and ``CORE_V6_Q`` = ``0xf``.  Names sharing a value are aliases for
the same encoded microarchitecture field.


Kernel limits and storage geometry
----------------------------------

.. list-table:: Kernel limits
   :header-rows: 1

   * - Definition
     - Value or meaning
   * - ``KERNEL_STACK_SIZE``
     - 512 bytes per hardware thread
   * - ``MAX_PRIOS``
     - 256 scheduler priority levels
   * - ``MAX_PRIO``
     - 255, the numerically largest priority
   * - ``BEST_PRIO``
     - 0, the numerically best priority
   * - ``MAX_VM_INTS``
     - 65535 virtual interrupts
   * - ``MAX_BOOT_CONTEXTS``
     - Number of boot contexts; defaults to 1 and may be overridden
   * - ``INTS_PER_BOOT_CONTEXT``
     - 32 interrupts per boot context
   * - ``BOOT_STACK_SIZE``
     - 128 bytes
   * - ``ALLOC_UNIT``
     - 8 words per smallest allocator block
   * - ``ALLOC_NUNITS``
     - 1024 allocator units
   * - ``DEFAULT_ALLOC_HEAP_SIZE``
     - ``ALLOC_NUNITS * ALLOC_UNIT + ALLOC_UNIT`` words, including the leading
       allocation tag/alignment unit
   * - ``CACHEIDX_MAX``
     - 2048 cache indices
   * - ``WAYS_MAX``
     - 16 cache ways
   * - ``SETS_MAX``
     - Maximum 32 KiB/32-byte-line set count derived using ``WAYS_MAX``
   * - ``MAX_TRACE_LEVEL``
     - 0; retained trace-level limit
   * - ``DEFAULT_TRACE_ENTRIES``
     - 16 retained default trace entries
   * - ``LOGBUF_PER_HT_SIZE``
     - Per-hardware-thread log buffer size; defaults to 4096 bytes and may be
       overridden
   * - ``KERNEL_CRASH_TCM_ADDR``
     - ``0xff070000`` when ``CRASH_DEBUG`` is enabled


Fixed virtual mappings and subsystem layout
-------------------------------------------

Page-size values in this section are hardware TLB size encodings, not byte
counts.

.. list-table:: Kernel virtual regions
   :header-rows: 1

   * - Definition
     - Value and purpose
   * - ``TEMP_MAP_VA``
     - ``0xff800000``; base of the temporary mapping window
   * - ``TEMP_MAP_PG_SIZE``
     - ``SIZE_4K``; default temporary mapping page-size encoding
   * - ``GPIO_VA``
     - ``0xff7f8000``; fixed GPIO mapping
   * - ``GPIO_PG_SIZE``
     - ``SIZE_4K``
   * - ``GPIO_PG_MASK``
     - Address mask derived from ``PAGE_BITS`` and ``GPIO_PG_SIZE``
   * - ``DEVICE_PAGE_SIZE``
     - ``SIZE_1M``; subsystem device-page encoding
   * - ``Q6_SS_BASE_VA``
     - ``0xffc00000``; virtual subsystem public-register base
   * - ``QDSP6SS_PUB_PRIV_OFFSET``
     - ``0x80000`` between subsystem public and private regions
   * - ``L2CFG_BASE_VA``
     - ``0xffd00000``; L2/configuration register mapping
   * - ``ANGEL_VA``
     - ``0xffe00000``; Angel mapping
   * - ``ANGEL_PG_SIZE``
     - ``SIZE_4K``

For ARCHV >= 5, ``QDSP6SS_PRIV_BASE_MSS`` is ``0xfc900000`` and
``QDSP6SS_PRIV_BASE_LPASS`` is ``0xfe280000``.  ``L2VIC_OFFSET`` is
``0x10000`` on supported families.  ``TIMER_OFFSET`` is ``0x4000`` on ARCHV 4
and ``0x20000`` on ARCHV >= 5.


Translation and page geometry
-----------------------------

.. list-table:: Translation definitions
   :header-rows: 1

   * - Definition
     - Value or meaning
   * - ``MAX_TLB_ENTRIES``
     - 192 regular hardware TLB entries supported by this interface
   * - ``TLB_ENTRY_C_BITS``
     - Cache-control field starts at bit 24
   * - ``TLB_ENTRY_GLOBAL_BIT``
     - Global flag at bit 62
   * - ``TLB_ENTRY_VALID_BIT``
     - Valid flag at bit 63
   * - ``DMA_TLB_START``
     - Absolute DMA TLB index base 512
   * - ``ASID_BITS``
     - 7 ASID bits
   * - ``MAX_ASIDS``
     - 128 ASIDs, derived as ``1 << ASID_BITS``
   * - ``STLB_MAX_SETS_LOG2``
     - 11
   * - ``STLB_MAX_SETS``
     - 2048 maximum STLB sets
   * - ``STLB_MULT``
     - 2 allocation multiplier
   * - ``STLB_MAX_WAYS``
     - 4 ways
   * - ``STLB_SHIFT``
     - 16, optimizing the software TLB index for 64 KiB pages
   * - ``STLB_ENTRIES``
     - ``STLB_MAX_SETS * STLB_MAX_WAYS * STLB_MULT`` (16384 entries)
   * - ``PAGE_SIZE``
     - Base page bytes, ``1 << PAGE_BITS``
   * - ``H2K_PAGESIZE``
     - Kernel page bytes, ``1 << (PAGE_BITS + 2 * H2K_KERNEL_PGSIZE)``
   * - ``PHYSREAD_HI_SHIFT``
     - 11; shift used to form the high portion of a physical read address


Interrupt namespace
-------------------

The interrupt namespace reserves L1 interrupts 0-31 and provides 480 L2
interrupts beginning at 32.

.. list-table:: Interrupt topology
   :header-rows: 1

   * - Definition
     - Value or meaning
   * - ``PERCPU_INTERRUPTS``
     - 16 per-VCPU virtual interrupts
   * - ``H2K_L2_CONTROL``
     - 1; L2 interrupt-controller support is present
   * - ``L2_INTERRUPT_START``
     - 32
   * - ``L2_CORE_INTERRUPT``
     - 2 on ARCHV >= 65, otherwise 31
   * - ``L1_INTERRUPTS``
     - 8 on ARCHV >= 65, otherwise 32
   * - ``MAX_L2_INTERRUPTS``
     - 480
   * - ``MAX_INTERRUPTS``
     - 512 physical interrupt numbers
   * - ``L1_FIRST_AVAILABLE_INTERRUPT``
     - 7; interrupts 3-5 are reserved for additional L2VIC instances
   * - ``L2_FIRST_AVAILABLE_INTERRUPT``
     - 32
   * - ``VM_IPI_INT``
     - Reserved VM IPI interrupt 0
   * - ``RESCHED_INT``
     - Reserved reschedule interrupt 1
   * - ``CLUSTER_RESCHED_INT``
     - Reserved cluster-reschedule interrupt 6

``VM_IPI_INTMASK``, ``RESCHED_INT_INTMASK``, and
``CLUSTER_RESCHED_INT_INTMASK`` are the one-bit masks derived from their
corresponding interrupt numbers.

The legacy timer interrupt constants are ``TIMER_INT_CORE_V4`` = 50 and
``TIMER_INT_CORE_V5``, ``TIMER_INT_CORE_V60``, and ``TIMER_INT_CORE_V61`` = 34.


MODECTL, SSR, CCR, and VWCTRL fields
-----------------------------------

.. c:macro:: BITS_MASK(NBITS, START)

Produces an ``u32_t`` mask containing ``NBITS`` one-bits beginning at
``START``.  It is used to derive the extension-control field masks.

``MODECTL_E_MASK`` is ``0x0000ffff``.  Waiting-thread bits begin at
``MODECTL_W_BITS`` = 16 and are selected by ``MODECTL_W_MASK`` =
``0xffff0000``.

The basic SSR bit positions are ``SSR_IE_BIT`` = 18, ``SSR_GUEST_BIT`` = 19,
and ``SSR_SS_BIT`` = 30.

.. list-table:: Extension-control fields
   :header-rows: 1

   * - Field
     - Position and width
     - Derived mask
   * - ``SSR_XA_BITS`` / ``SSR_XA_NBITS``
     - bit 27, width 3
     - ``SSR_XA_BITS_MASK``
   * - ``SSR_XE_BIT`` / ``SSR_XE_NBITS``
     - bit 31, width 1
     - ``SSR_XE_BIT_MASK``
   * - ``SSR_XE2_BIT`` / ``SSR_XE2_NBITS``
     - bit 26, width 1
     - ``SSR_XE2_BIT_MASK``
   * - ``CCR_XE3_BIT`` / ``CCR_XE3_NBITS``
     - bit 28, width 1
     - ``CCR_XE3_BIT_MASK``
   * - ``CCR_XA3_BITS`` / ``CCR_XA3_NBITS``
     - bit 21, width 1
     - ``CCR_XA3_BITS_MASK``
   * - ``CCR_COPROC_BIT``
     - bit 3, width 1
     - ``CCR_COPROC_BIT_MASK``
   * - ``CCR_L2CP_BITS`` / ``CCR_L2CP_NBITS``
     - bit 6, width 2
     - No separate mask macro

``VWCTRL_EN_BIT`` is bit 31.  Its high field starts at ``VWCTRL_HI_BITS`` = 16,
has ``VWCTRL_HI_NBITS`` = 12, and is selected by
``VWCTRL_HI_BITS_MASK``.  Its low field starts at ``VWCTRL_LO_BITS`` = 0 and
has ``VWCTRL_LO_NBITS`` = 12.

``CLUSTER_SCHED_MIN_COPROCS`` requires at least two coprocessors per cluster,
and ``CLUSTER_MAX_CLUSTERS`` limits the processor to four scheduler clusters.


Initial thread register state
-----------------------------

.. list-table:: Boot register defaults
   :header-rows: 1

   * - Definition
     - Selection
   * - ``BOOT_THREAD_USR``
     - ``0`` on ARCHV <= 4; ``0x00056000`` on ARCHV 5 and 60;
       ``0x00057c00`` on later families
   * - ``BOOT_THREAD_SSR``
     - ``0x01c60000`` with ``SSR_GUEST_BIT`` set
   * - ``BOOT_THREAD_CCR``
     - ``0x00130000`` on ARCHV 60; ``0x00170000`` otherwise
   * - ``BOOT_THREAD_GPUGP``
     - 0, ensuring the default user global pointer/thread-ID behavior


Coprocessor power-control registers
-----------------------------------

These definitions exist when ``HAVE_EXTENSIONS`` is enabled.

For ARCHV < 65, ``QDSP6SS_CP_CLK_CTL`` = ``0xfc``,
``QDSP6SS_CP_RESET`` = ``0xf8``, and ``QDSP6SS_CP_PWR_CTL`` = ``0xf0``.
``QDSP6SS_CP_CLK_CTL_DISABLE`` / ``QDSP6SS_CP_CLK_CTL_ENABLE`` are 0/1 and
``QDSP6SS_CP_RESET_ASSERT`` / ``QDSP6SS_CP_RESET_DEASSERT`` are 1/0.  The V60 power sequences are
``QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V60`` = ``0x011fffff``,
``QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF_V60`` = ``0x010fffff``, and
``QDSP6SS_CP_PWR_CTL_POWER_OFF_V60`` = ``0x001fffff``.  The V62 equivalents are
``QDSP6SS_CP_PWR_CTL_CLAMP_IO_ON_V62`` = 3,
``QDSP6SS_CP_PWR_CTL_CLAMP_IO_OFF_V62`` = 2, and
``QDSP6SS_CP_PWR_CTL_POWER_OFF_V62`` = 1.

For ARCHV >= 65, the principal register offsets are:

.. list-table:: ARCHV 65+ extension registers
   :header-rows: 1

   * - Register definition
     - Offset and fields
   * - ``QDSP6SS_CP_PWR_CTL``
     - ``0x500``; ``QDSP6SS_CP_PWR_CTL_CLAMP_IO_BIT`` = 0 and
       ``QDSP6SS_CP_PWR_CTL_CLAMP_QMC_MEM_BIT`` = 4
   * - ``QDSP6SS_CP_RESET``
     - ``0x504``; ASSERT = 1 and DEASSERT = 0
   * - ``QDSP6SS_CP_CLK_CTL``
     - ``0x508``; DISABLE = 0 and ENABLE = 1
   * - ``QDSP6SS_CP_BHS_CFG``
     - ``0x510``; ``QDSP6SS_CP_BHS_CFG_BHS_ON_BIT`` = 0
   * - ``QDSP6SS_CP_BHS_CMD``
     - ``0x514``; ``QDSP6SS_CP_BHS_CMD_UPDATE_BIT`` = 0
   * - ``QDSP6SS_CP_BHS_STATUS``
     - ``0x518``; ``QDSP6SS_CP_BHS_STATUS_BHS_ON_BIT`` = 0
   * - ``QDSP6SS_CPMEM_CFG``
     - ``0x520``; ``QDSP6SS_CPMEM_CFG_VTCM_POWER_ON`` = 3
   * - ``QDSP6SS_CPMEM_CMD``
     - ``0x524``; update bits
       ``QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_NRET_N_BIT`` = 0 and
       ``QDSP6SS_CPMEM_CMD_UPDATE_VTCM_SLP_RET_N_BIT`` = 1
   * - ``QDSP6SS_CPMEM_STATUS``
     - ``0x528``; status bits
       ``QDSP6SS_CPMEM_STATUS_VTCM_SLP_NRET_N_BIT`` = 0 and
       ``QDSP6SS_CPMEM_STATUS_VTCM_SLP_RET_N_BIT`` = 1

On ARCHV >= 68, the HMX resource-state coordinator definitions are
``QDSP6SS_RSCC_RSC_SEQ_BUSY_DRV0`` = ``0x30404`` with
``QDSP6SS_RSCC_RSC_SEQ_BUSY_DRV0_STATUS_BIT`` = 0,
``QDSP6SS_RSCC_RSC_SEQ_OVERRIDE_TRIGGER_DRV0`` = ``0x30460`` with
``QDSP6SS_RSCC_RSC_SEQ_OVERRIDE_TRIGGER_DRV0_TRIGGER_BIT`` = 0, and
``QDSP6SS_RSCC_RSC_SEQ_OVERRIDE_TRIGGER_START_ADDR_DRV0`` = ``0x30464`` with
``QDSP6SS_RSCC_RSC_SEQ_OVERRIDE_TRIGGER_START_ADDR_DRV0_ADDRESS_BITMASK`` =
``0x3ff``.


Hardware configuration-table schema
-----------------------------------

``CFG_TABLE_SHIFT`` = 16.  Configuration-table readers form the base address
from ``CFGBASE << CFG_TABLE_SHIFT`` and add one of the byte offsets below.

.. list-table:: Configuration-table base and topology entries
   :header-rows: 1

   * - Definition
     - Offset
   * - ``CFG_TABLE_L2TCM``
     - ``0x00``
   * - ``CFG_TABLE_SSBASE``
     - ``0x08``
   * - ``CFG_TABLE_CORECFG_BASE``
     - ``0x0c``
   * - ``CFG_TABLE_L2REGS``
     - ``0x10``
   * - ``CFG_TABLE_CLADEREGS``
     - ``0x24``
   * - ``CFG_TABLE_FASTL2VIC_BASE``
     - ``0x28``
   * - ``CFG_TABLE_ECC_BASE``
     - ``0x4c``
   * - ``CFG_TABLE_CLADE2REGS``
     - ``0x60``
   * - ``CFG_TABLE_CORE_ID`` / ``CFG_TABLE_CORE_COUNT``
     - ``0x70`` / ``0x74``
   * - ``CFG_TABLE_AXIM2_SECONDARY_BASE``
     - ``0xdc``
   * - ``CFG_TABLE_UNIT_CONFIG_REG_BASE``
     - ``0x108``

.. list-table:: Configuration-table memory and translation entries
   :header-rows: 1

   * - Definition
     - Offset
   * - ``CFG_TABLE_JTLB_SIZE``
     - ``0x2c``
   * - ``CFG_TABLE_VTCM_BASE`` / ``CFG_TABLE_VTCM_SIZE``
     - ``0x38`` / ``0x3c``
   * - ``CFG_TABLE_L2TAG_SIZE`` / ``CFG_TABLE_L2ARRAY_SIZE``
     - ``0x40`` / ``0x44``
   * - ``CFG_TABLE_HTHREADS_MASK``
     - ``0x48``
   * - ``CFG_TABLE_L2_LINE_SZ``
     - ``0x50``
   * - ``CFG_TABLE_L2ITCM_SZ`` / ``CFG_TABLE_L2ITCM_BASE``
     - ``0x58`` / ``0x5c``
   * - ``CFG_TABLE_L1D_SZ`` / ``CFG_TABLE_L1I_SZ``
     - ``0xa4`` / ``0xa8``
   * - ``CFG_TABLE_L1D_WRITE_POLICY``
     - ``0xac``
   * - ``CFG_TABLE_VTCM_BANK_WIDTH``
     - ``0xb0``
   * - ``CFG_TABLE_VICTIM_CACHE_MODE``
     - ``0xbc``
   * - ``CFG_TABLE_DMA_JTLB_SIZE``
     - ``0xf0``

.. list-table:: Configuration-table feature and coprocessor entries
   :header-rows: 1

   * - Definition
     - Offset
   * - ``CFG_TABLE_COPROC_TYPE`` / ``CFG_TABLE_COPROC_CONTEXTS``
     - ``0x30`` / ``0x34``
   * - ``CFG_TABLE_AUDIO_EXT``
     - ``0x54``
   * - ``CFG_TABLE_DTM_VERSION`` / ``CFG_TABLE_DMA_VERSION``
     - ``0x64`` / ``0x68``
   * - ``CFG_TABLE_COPROC_VLENGTH``
     - ``0x6c``
   * - ``CFG_TABLE_HMX_INT8_RATE`` / ``CFG_TABLE_HMX_FP16_RATE``
     - ``0x78`` / ``0x7c``
   * - ``CFG_TABLE_VX2XMODE``
     - ``0x80``
   * - ``CFG_TABLE_HLX_CONTEXTS`` / ``CFG_TABLE_HLX_LENGTH``
     - ``0x84`` / ``0x88``
   * - ``CFG_TABLE_ACD_PRESET`` / ``CFG_TABLE_MND_PRESET``
     - ``0x9c`` / ``0xa0``
   * - ``CFG_TABLE_ATOMICS_VERSION``
     - ``0xb4``
   * - ``CFG_TABLE_HVX_IEEE_PRESENT``
     - ``0xb8``
   * - ``CFG_TABLE_CORECFG_PRESENT``
     - ``0xd4``

The remaining interconnect/domain entries are
``CFG_TABLE_CONSISTENCY_DOMAIN`` = ``0xc4``,
``CFG_TABLE_CAPACITY_DOMAIN`` = ``0xc8``,
``CFG_TABLE_AXI3_LOWADDR`` = ``0xcc``, and
``CFG_TABLE_AXIM_DATA_WIDTH`` = ``0xe0``.

Unit blocks begin with ``CFG_TABLE_UNIT_ID_OFFSET`` = 0,
``CFG_TABLE_UNIT_SUBID_OFFSET`` = 4, and ``CFG_TABLE_UNIT_NEXT_OFFSET`` = 8.
The ``CFG_TABLE_COPROC_TYPE_HVX_MASK``,
``CFG_TABLE_COPROC_TYPE_SILVER_MASK``,
``CFG_TABLE_COPROC_TYPE_HMX_MASK``, and
``CFG_TABLE_COPROC_TYPE_HLX_MASK`` values are ``0x1``, ``0x2``, ``0x4``, and
``0x10`` respectively.


Legacy feature fallbacks
------------------------

For cores that do not report these values through the configuration table,
``EXT_HVX_CONTEXTS`` defaults to 4, ``EXT_HVX_VTCM_BANK_WIDTH`` to 16,
``EXT_HVX_VTCM_OFFSET`` to ``0x200000``, ``EXT_HVX_VTCM_SIZE`` to 256, and
``EXT_HVX_MAX_VLENGTH`` to 128 bytes.  ``LIMIT_L1D_SZ`` and ``LIMIT_L1I_SZ``
default to 16 KiB.


L2 register layout and bounds
-----------------------------

The legacy coprocessor energy registers are
``L2REGS_COPROC_EGY_CFG`` = ``0x28`` with
``L2REGS_COPROC_EGY_CFG_DEFAULT_V62`` = ``0xc3e03ff5``, and
``L2REGS_COPROC_EGY_WEIGHTS`` = ``0x2c`` with
``L2REGS_COPROC_EGY_WEIGHTS_DEFAULT_V62`` = ``0xa61686e0``.

.. list-table:: L2 QoS register offsets
   :header-rows: 1

   * - Definition
     - Offset
   * - ``L2REGS_QOS_MODE``
     - ``0x100``; ``L2REGS_QOS_MODE_TL_BIT`` = 7
   * - ``L2REGS_QOS_MAX_TRANS``
     - ``0x104``
   * - ``L2REGS_QOS_ISSUE_RATE``
     - ``0x108``
   * - ``L2REGS_QOS_DANGER_ISSUE_RATE``
     - ``0x10c``
   * - ``L2REGS_QOS_SCOREBOARD_WATERMARK``
     - ``0x110``
   * - ``L2REGS_QOS_SYS_PRI``
     - ``0x114``
   * - ``L2REGS_QOS_DANGER_0``, ``L2REGS_QOS_DANGER_1``,
       ``L2REGS_QOS_DANGER_2``, ``L2REGS_QOS_DANGER_3``
     - ``0x1000``, ``0x2000``, ``0x3000``, ``0x4000``
   * - ``L2REGS_MAX``
     - ``L2REGS_QOS_DANGER_3``

The scoreboard watermark has three 8-bit fields.  The L2 field uses
``L2REGS_QOS_SCOREBOARD_WATERMARK_WM_L2_BITS`` = 0 and
``L2REGS_QOS_SCOREBOARD_WATERMARK_WM_L2_LEN`` = 8; the low field uses
``L2REGS_QOS_SCOREBOARD_WATERMARK_WM_LO_BITS`` = 8 and
``L2REGS_QOS_SCOREBOARD_WATERMARK_WM_LO_LEN`` = 8; the high field uses
``L2REGS_QOS_SCOREBOARD_WATERMARK_WM_HI_BITS`` = 16 and
``L2REGS_QOS_SCOREBOARD_WATERMARK_WM_HI_LEN`` = 8.  Their derived masks are
``L2REGS_QOS_SCOREBOARD_WATERMARK_WM_L2``,
``L2REGS_QOS_SCOREBOARD_WATERMARK_WM_LO``, and
``L2REGS_QOS_SCOREBOARD_WATERMARK_WM_HI``.  The
``L2REGS_QOS_SCOREBOARD_WATERMARK_DEFAULT`` places 24 in the L2 and low fields
and 1 in the high field.

``CLADEREGS_MAX`` bounds the CLADE register window at
``CLADE_REG_PD_CHUNK * CLADE_NUM_PDS + 0x1c``.


Core-configuration register windows
-----------------------------------

``CORECFG_BLOCK_SIZE`` is ``0x1000`` and ``CORECFG_BLOCK_COUNT`` is 4.  The
block bases are ``CORECFG_ETM_BASE`` = 0,
``CORECFG_LIVELOCK_BASE`` = ``1 * CORECFG_BLOCK_SIZE``,
``CORECFG_STRIDE_PREFETCHER_BASE`` = ``2 * CORECFG_BLOCK_SIZE``, and
``CORECFG_DPM_VOLTLMTMGMT_BASE`` = ``3 * CORECFG_BLOCK_SIZE``.
``CORECFG_DPM_VOLTLMTMGMT_MAX`` is the final aligned word in its block,
``CORECFG_BLOCK_SIZE - 4``.

The corresponding presence bits are ``CORECFG_PRESENT_ETM_MASK`` = ``0x1``,
``CORECFG_PRESENT_LIVELOCK_MASK`` = ``0x2``,
``CORECFG_PRESENT_STRIDE_PREFETCHER_MASK`` = ``0x4``, and
``CORECFG_PRESENT_DPM_VOLTLMTMGMT_MASK`` = ``0x8``.
