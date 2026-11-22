:mod:`hw` -- Hardware interface helpers
===========================================

.. module:: hw

This module wraps Hexagon special registers, interrupt instructions, cache and
TLB synchronization instructions, hardware locks, and assembly helpers.  The
inline functions operate on the current processor unless a thread or mask is
explicitly supplied.


Interrupt and assignment helpers
---------------------------------

``ciad(mask)`` clears interrupt-auto-disable bits; ``siad(mask)`` sets them;
``swi(mask)`` raises software interrupts.  ``iassignw(intno, threadmask)``
writes interrupt assignment and ``iassignr(intno)`` reads it.
``hthreads_mask()`` writes and reads the assignment mask for all threads, and
``get_hthreads()`` returns the hardware-thread count (popcount on ARCHV >= 5;
the architecture-specific fallback on ARCHV 4).

``change_imask(thread, imask)`` writes a thread's IMASK and
``get_imask(thread)`` reads it.  ``resched_int()`` and
``cluster_resched_int()`` raise the configured reschedule interrupts.
``highprio_imask(hthread)`` and ``lowprio_imask(hthread)`` install the masks
used for high- and low-priority scheduling; the current implementation writes
all ones and zero respectively.


Special-register accessors
--------------------------

The following accessors read or write their named special register:

``H2K_get_ssr`` / ``H2K_set_ssr``, ``H2K_get_syscfg`` / ``H2K_set_syscfg``,
``H2K_get_livelock`` / ``H2K_set_livelock``, ``H2K_get_turkey`` /
``H2K_set_turkey``, ``H2K_get_duck`` / ``H2K_set_duck``,
``H2K_get_chicken`` / ``H2K_set_chicken``, ``H2K_get_rgdr`` /
``H2K_set_rgdr``, ``H2K_get_vwctrl`` / ``H2K_set_vwctrl``,
``H2K_get_tid_reg`` / ``H2K_set_tid_reg``, ``H2K_get_elr`` /
``H2K_set_elr``, ``H2K_get_gp`` / ``H2K_set_gp``, and
``H2K_get_ccr`` / ``H2K_set_ccr``.  ``H2K_get_cfgbase`` reads CFGBASE;
``H2K_get_modectl`` reads MODECTL; ``get_hwtnum`` reads HTID; and
``H2K_get_sgp`` reads the saved guest pointer.

``H2K_get_pcycle_reg`` and ``H2K_get_timer_reg`` read processor cycles and the
hardware timer.  The timer helper returns zero on architectures where the
register is unavailable.

``H2K_get_ipend`` and ``H2K_get_iad`` read pending and interrupt-auto-disable
state, with architecture-specific register selection.  ``H2K_clear_ipend``
clears pending interrupts for a hardware-thread mask.


Global interrupt and synchronization helpers
---------------------------------------------

``H2K_clear_gie`` clears and ``H2K_set_gie`` sets the global interrupt-enable
bit in SYSCFG, followed by an instruction synchronization.  ``H2K_isync``,
``H2K_syncht``, and ``H2K_ickill`` issue their corresponding synchronization or
kill instructions.  ``H2K_l2kill`` and, on ARCHV >= 60, ``H2K_l2gcleaninv`` /
``H2K_l2gclean`` perform L2 maintenance.

``H2K_mutex_lock_k0`` / ``H2K_mutex_unlock_k0`` operate the K0 hardware lock;
``H2K_mutex_lock_tlb`` / ``H2K_mutex_unlock_tlb`` operate the TLB lock.
``H2K_gregs_save(me)`` copies guest registers into the context and
``H2K_gregs_restore(me)`` writes them back.

``BKL_LOCK`` and ``BKL_UNLOCK`` select the software ticket BKL or K0 lock.
``IS_BKL_LOCKED`` is a diagnostic check, not a production synchronization
primitive.


Cache, TLB, and memory helpers
------------------------------

``H2K_dccleana(ptr)`` and ``H2K_dccleaninva(ptr)`` clean or clean-and-invalidate
a data-cache line.  ``H2K_l2locka(ptr)`` attempts to lock an L2 line and
returns the hardware predicate result; ``H2K_l2unlock()`` releases the L2
lock.

On ARCHV >= 68, ``H2K_dmtlbsynch()`` synchronizes DMA TLB state,
``H2K_dmcfgrd(index)`` reads DMA configuration, and
``H2K_dmcfgwr(index, data)`` writes it.


Thread control
--------------

.. c:function:: void H2K_start_threads(unsigned int mask)

Starts the hardware threads selected by ``mask`` using the ``start``
instruction.


Configuration-table helpers
---------------------------

.. c:data:: H2K_unit_offsets

The generated offset table maps configuration unit type, subtype, and entry to
the physical configuration-table offset.

.. c:function:: static inline u32_t H2K_cfg_table(u32_t entry)

	:param entry: Configuration-table byte offset
	:returns: Word read from the configuration table

Reads a word at ``CFGBASE << CFG_TABLE_SHIFT`` plus ``entry`` through
:c:func:`H2K_mem_physread_word()`.


.. c:function:: static inline u32_t H2K_cfg_table_unit_entry(u32_t unit, h2_cfg_unit_entry entry)


	:param unit: Unit offset in the configuration table
	:param entry: Unit entry selector
	:returns: Requested unit-entry word

Reads the unit type and subtype, looks up the corresponding offset in
``H2K_unit_offsets``, and reads that unit entry through
:c:func:`H2K_mem_physread_word()`.

H2K_get_bestwait
----------------

.. c:function::  static inline u32_t H2K_get_bestwait()

Description
~~~~~~~~~~~

This function retrieves the contents of the BESTWAIT register.  The hardware
resets BESTWAIT to 0x1FF after it raises the reschedule interrupt, so reading it
back indicates whether the hardware has already fired since it was last
programmed.  See the hardware reschedule interrupt.


H2K_set_bestwait
----------------

.. c:function::  static inline void H2K_set_bestwait(u32_t priority_mask)

	:param priority_mask:  Priority value to program into BESTWAIT

Description
~~~~~~~~~~~

This function programs the BESTWAIT register with the priority of the highest
priority task waiting to run.  The hardware raises the reschedule interrupt when
any hardware thread's effective STID.PRIO is worse than this value.  See the
hardware reschedule interrupt in conjunction with the SCHEDCFG register.


H2K_get_schedcfg
----------------

.. c:function::  static inline u32_t H2K_get_schedcfg()

Description
~~~~~~~~~~~

This function retrieves the contents of the SCHEDCFG register, which configures
the hardware reschedule interrupt feature (SCHEDCFG.EN enables it, SCHEDCFG.INTNO
selects the interrupt raised).


H2K_set_schedcfg
----------------

.. c:function::  static inline void H2K_set_schedcfg(u32_t val)

	:param val:  Value to write to the SCHEDCFG register

Description
~~~~~~~~~~~

This function writes the SCHEDCFG register, used to enable the hardware
reschedule interrupt feature and select which interrupt it raises.  See the
hardware reschedule interrupt in conjunction with the BESTWAIT register.

