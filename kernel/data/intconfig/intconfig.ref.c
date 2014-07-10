/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <context.h>
#include <intconfig.h>
#include <resched.h>
#include <max.h>
#include <hexagon_protos.h>
#include <thread.h>
#include <hw.h>
#include <globals.h>
#include <vmipi.h>
#include <timer.h>
#include <idtype.h>
#include <intcontrol.h>
#include <passthru.h>

H2K_fastint_context H2K_fastint_contexts[MAX_HTHREADS];

#define FASTINT_TRAPMASK 0x9 /* ANGEL | FUTEX_RESUME */

void H2K_fastint();

#ifdef H2K_L2_CONTROL
static void H2K_fastint_enable_l2(u32_t whatint)
{
	volatile unsigned int *l2_enable = (void *)H2K_gp->l2_ack_base;
	whatint -= 32;
	l2_enable[(whatint)/32] = (1 << (whatint & 0x1f));
}

static void H2K_fastint_disable_l2(u32_t whatint)
{
	volatile unsigned int *l2_disable = (void *)(((unsigned int)H2K_gp->l2_int_base) + 0x180);
	whatint -= 32;
	l2_disable[(whatint)/32] = (1 << (whatint & 0x1f));
}

#else
static inline void H2K_fastint_enable_l2(int whatint) {}
static inline void H2K_fastint_disable_l2(int whatint) {}
#endif

static void H2K_fastint_disable(u32_t whatint)
{
	u32_t siad_intmask;
	siad_intmask = 1<<whatint;
	if (whatint < 32) {
#if ARCHV >= 4
		siad(siad_intmask);
#endif
		return;
	} else H2K_fastint_disable_l2(whatint);
}

static void H2K_fastint_enable(u32_t whatint)
{
	u32_t ciad_intmask;
	if (whatint < 32) {
		ciad_intmask = 1<<whatint;
#if ARCHV <= 3
		ciad_intmask = Q6_R_brev_R(ciad_intmask);
#endif
		ciad(ciad_intmask);
	} else H2K_fastint_enable_l2(whatint);
}

void H2K_register_fastint(u32_t whatint, int (*fastint_handler)(u32_t x), H2K_thread_context *me)
{
	H2K_inthandler_t tmp;

	if (fastint_handler == NULL) { /* deregister */
		H2K_fastint_disable(whatint);
		H2K_gp->inthandlers[whatint].raw = 0;
	} else {
		/* write atomically */
		tmp.param = fastint_handler;
		tmp.handler = H2K_fastint;
		H2K_gp->inthandlers[whatint].raw = tmp.raw;
		H2K_fastint_enable(whatint);

		H2K_gp->fastint_gp = (u32_t)(me->gp);
	}
}

void H2K_register_passthru(u32_t phys_int, H2K_id_t id, u32_t virt_int) {

	H2K_inthandler_t tmp;

	if (id.vmidx == 0) { /* deregister */
		H2K_intcontrol_disable(phys_int);
		H2K_gp->inthandlers[phys_int].raw = 0;
	} else {
		/* write atomically */
		if (virt_int >= 0x1 << H2K_ID_VINT_BITS) { // too big
			id.reserved = 0; // look in vmblock; it will have been set up there
		} else {
			id.reserved = virt_int;
		}
		tmp.param = (void *)id.raw;
		tmp.handler = H2K_passthru;
		H2K_gp->inthandlers[phys_int].raw = tmp.raw;
		H2K_intcontrol_enable(phys_int);
	}
}

#if H2K_L2_CONTROL && ! defined NO_DEVICES
static void H2K_intconfig_l2_init(ssbase)
{

	int i;
	volatile unsigned int *intbase = H2K_gp->l2_int_base;

	u32_t spi_irq, spi_word, tlmm_irq, tlmm_word;

#if ARCHV >= 5
	u32_t spmi_arb_periph_irq, spmi_arb_periph_word, spmi_arb_ee_irq, spmi_arb_ee_word;
#endif

#if ARCHV == 4
	/*  8960 SPI and TLMM summary apparently are level (high) triggered */
#define SPI_IRQ		65
#define SPI_WORD	(SPI_IRQ/32)
#define TLMM_IRQ	38
#define TLMM_WORD	(TLMM_IRQ/32)

	spi_irq = SPI_IRQ;
	spi_word = SPI_WORD;
	tlmm_irq = TLMM_IRQ;
	tlmm_word = TLMM_WORD;
#endif

#if ARCHV >= 5
#define MSS_SPI_IRQ		245
#define MSS_SPI_WORD	(MSS_SPI_IRQ/32)
#define MSS_TLMM_IRQ	239
#define MSS_TLMM_WORD	(MSS_TLMM_IRQ/32)

#define MSS_SPMI_ARB_PERIPH_IRQ	75
#define MSS_SPMI_ARB_PERIPH_WORD	(MSS_SPMI_ARB_PERIPH_IRQ/32)
#define MSS_SPMI_ARB_EE_IRQ		76
#define MSS_SPMI_ARB_EE_WORD	(MSS_SPMI_ARB_EE_IRQ/32)

		/*  8974 SPI ("blsp1_qup1") and "summary_irq_sensors"  */
#define LPASS_SPI_IRQ		64
#define LPASS_SPI_WORD	(LPASS_SPI_IRQ/32)
#define LPASS_TLMM_IRQ	38
#define LPASS_TLMM_WORD	(LPASS_TLMM_IRQ/32)

#define LPASS_SPMI_ARB_PERIPH_IRQ	48
#define LPASS_SPMI_ARB_PERIPH_WORD	(LPASS_SPMI_ARB_PERIPH_IRQ/32)
#define LPASS_SPMI_ARB_EE_IRQ		49
#define LPASS_SPMI_ARB_EE_WORD	(LPASS_SPMI_ARB_EE_IRQ/32)

	/* Ew ew ew */
	if (ssbase == QDSP6SS_PRIV_BASE_MSS - QDSP6SS_PUB_PRIV_OFFSET) {
		spi_irq = MSS_SPI_IRQ;
		spi_word = MSS_SPI_WORD;
		tlmm_irq = MSS_TLMM_IRQ;
		tlmm_word = MSS_TLMM_WORD;
		spmi_arb_periph_irq = MSS_SPMI_ARB_PERIPH_IRQ;
		spmi_arb_periph_word = MSS_SPMI_ARB_PERIPH_WORD;
		spmi_arb_ee_irq = MSS_SPMI_ARB_EE_IRQ;
		spmi_arb_ee_word = MSS_SPMI_ARB_EE_WORD;
	
	} else {  // guess LPASS
		spi_irq = LPASS_SPI_IRQ;
		spi_word = LPASS_SPI_WORD;
		tlmm_irq = LPASS_TLMM_IRQ;
		tlmm_word = LPASS_TLMM_WORD;
		spmi_arb_periph_irq = LPASS_SPMI_ARB_PERIPH_IRQ;
		spmi_arb_periph_word = LPASS_SPMI_ARB_PERIPH_WORD;
		spmi_arb_ee_irq = LPASS_SPMI_ARB_EE_IRQ;
		spmi_arb_ee_word = LPASS_SPMI_ARB_EE_WORD;
	}
#endif

	for (i = 0; i < ((MAX_INTERRUPTS-32)/32); i++) {
		intbase[(0x100/4) + i] = 0x0; 		/* DISABLED */
		intbase[(0x280/4) + i] = 0xFFFFFFFF;	/* EDGE/level TRIGGERED */
		intbase[(0x300/4) + i] = 0x0;		/* Rising Edge / Level High */
		intbase[(0x400/4) + i] = 0xFFFFFFFF;	/* Interrupt Clear */

		/*  is there a "clear bit" around here?  */
		/*  There's probably a better way to do this.  */
		if (i == spi_word) {
			intbase[(0x280/4) + i] &= ~(1<<(spi_irq % 32));
		}
		if (i == tlmm_word) {
			intbase[(0x280/4) + i] &= ~(1<<(tlmm_irq % 32));
		}
#if ARCHV >= 5
		if (i == spmi_arb_periph_word) {
			intbase[(0x280/4) + i] &= ~(1<<(spmi_arb_periph_irq % 32));
		}
		if (i == spmi_arb_ee_word) {
			intbase[(0x280/4) + i] &= ~(1<<(spmi_arb_ee_irq % 32));
		}
#endif
	}
	ciad(0x80000000);				/* Enable L2 Interrupts */
}
#else
static inline void H2K_intconfig_l2_init(u32_t ssbase) {}
#endif

void H2K_intconfig_init(u32_t ssbase)
{
	int i;
	H2K_thread_context *tmp;
	for (i = 0; i < MAX_INTERRUPTS; i++) {
		H2K_gp->inthandlers[i].raw = 0;
	}
	H2K_gp->inthandlers[RESCHED_INT].handler = H2K_resched;
	H2K_gp->inthandlers[VM_IPI_INT].handler = H2K_vm_ipi_do;
	H2K_gp->inthandlers[TIMER_INT].handler = H2K_timer_int;
	for (i = 0; i < MAX_HTHREADS; i++) {
		tmp = &H2K_fastint_contexts[i].context;
		H2K_thread_context_clear(tmp);
		tmp->hthread = i;
		tmp->trapmask = FASTINT_TRAPMASK;
	}
	H2K_intconfig_l2_init(ssbase);
}

