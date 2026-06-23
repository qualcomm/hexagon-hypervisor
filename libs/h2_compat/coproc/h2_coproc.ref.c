/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_coproc.h>
#include <h2_alloc.h>

/* Number of context bits in a unit's context mask (one unsigned int). */
#define COPROC_BITS_PER_UNIT (sizeof(unsigned int) * 8)

static unsigned int *configs[CFG_TYPE_MAX][CFG_SUBTYPE_MAX] = {NULL};
static unsigned int nunits[CFG_TYPE_MAX][CFG_SUBTYPE_MAX] = {0};
static unsigned int *counts[CFG_TYPE_MAX][CFG_SUBTYPE_MAX][CFG_MAX] = {NULL};
static unsigned char *bitpos[CFG_TYPE_MAX][CFG_SUBTYPE_MAX][CFG_MAX] = {NULL};
static unsigned int oldstyle = 0;
static unsigned int init_done = 0;

/* Record the bit positions of the set bits in `bits`, in ascending order, into `out`. */
static void coproc_build_bitpos(unsigned int bits, unsigned char *out) {
	unsigned int pos = 0;
	unsigned int k = 0;

	while (bits) {
		if (bits & 0x1) {
			out[k++] = (unsigned char)pos;
		}
		bits >>= 1;
		pos++;
	}
}

static int coproc_grow(void **slot, int newsize) {
	void *p = h2_realloc(*slot, newsize);

	if (NULL == p) {
		return -1;
	}
	*slot = p;
	return 0;
}

int h2_coproc_count(h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int unit_mask) {
	unsigned int i;
	int ret = 0;
	
	if (oldstyle) return -1;

	if (-1 == unit_mask) {  // sum of all
		for (i = 0; i < nunits[type][subtype]; i++) {
			ret += counts[type][subtype][entry_type][i];
		}
		return ret;
	} else {
		for (i = 0; i < sizeof(unsigned int) * 8; i++) {
			if (i >= nunits[type][subtype]) {  // end of range
				break;
			}
			if (unit_mask & (0x1 << i)) {
				ret += counts[type][subtype][entry_type][i];
			}
		}
	}
	return ret;
}

static int setbits(h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int select, unsigned int enable) {
	switch(type) {
	case CFG_TYPE_VXU0:
		switch (subtype) {
		case CFG_SUBTYPE_VXU0:
			switch (entry_type) {
			case CFG_HVX_CONTEXTS:
				return h2_hwconfig_extbits(select, enable);
			case CFG_HLX_CONTEXTS:
				return h2_hwconfig_set_hlxbits(select, enable);
			case CFG_HMX_CONTEXTS:
				return h2_hwconfig_set_hmxbits(enable, select);
			default:
				return -1;
			}
		default:
			return -1;
		}
	default:
		return -1;
	}
}

int h2_coproc_set(h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int unit_mask, unsigned int num, unsigned int enable) {

	unsigned int idx = 0;  // unit being considered
	unsigned int ncontexts;
	unsigned int xa;
	unsigned int *entry;

	if (oldstyle || !enable) {
		return setbits(type, subtype, entry_type, num, enable);
	}

	num += 1;  // num == number of existing contexts left to find

	while (num && idx < nunits[type][subtype]) {  // still searching and in range
		if (unit_mask & (0x1 << idx)) {  // unit selected
			entry = &configs[type][subtype][idx * CFG_MAX];
			ncontexts = counts[type][subtype][entry_type][idx];
			if (num <= ncontexts) {  // requested context # is in this unit
				xa = bitpos[type][subtype][entry_type][idx * COPROC_BITS_PER_UNIT + (num - 1)];

				setbits(type, subtype, entry_type, xa, enable);
				h2_hwconfig_set_coprocbits(entry[CFG_VXU_UNIT_ID]);  // FIXME: need to get the ID field for this unit type
				return 0;
			} else {
				num -= ncontexts;  // skip all the contexts in this unit
			}
		}
		idx++;  // check next unit
	}
	// requested a non-existent context
	return -1;
}

/*
 * Populate the per-unit context count and logical->physical bit map for one
 * entry type (HVX/HLX/HMX) of a unit.  Grows the count array to cover `idx`
 * and the bitpos array to cover this unit's COPROC_BITS_PER_UNIT slot.
 */
static int init_entry_contexts(h2_coproc_type_t type, h2_coproc_subtype_t subtype, h2_cfg_unit_entry entry_type, unsigned int idx, unsigned int bits) {
	if (-1 == coproc_grow((void **)&counts[type][subtype][entry_type], (idx + 1) * sizeof(unsigned int))) {
		return -1;
	}
	counts[type][subtype][entry_type][idx] = Q6_R_popcount_P(bits);

	if (-1 == coproc_grow((void **)&bitpos[type][subtype][entry_type], (idx + 1) * COPROC_BITS_PER_UNIT * sizeof(unsigned char))) {
		return -1;
	}
	coproc_build_bitpos(bits, &bitpos[type][subtype][entry_type][idx * COPROC_BITS_PER_UNIT]);

	return 0;
}

static int init_entry_vxu0(unsigned int unit, h2_coproc_type_t type, h2_coproc_subtype_t subtype, unsigned int idx) {
	unsigned int *entry;

	entry = &configs[type][subtype][idx * CFG_MAX];
	entry[CFG_VXU_UNIT_ID] = h2_info_unit(unit, CFG_VXU_UNIT_ID);

	entry[CFG_HVX_CONTEXTS] = h2_info_unit(unit, CFG_HVX_CONTEXTS);
	if (-1 == init_entry_contexts(type, subtype, CFG_HVX_CONTEXTS, idx, entry[CFG_HVX_CONTEXTS])) {
		return -1;
	}

	entry[CFG_HLX_CONTEXTS] = h2_info_unit(unit, CFG_HLX_CONTEXTS);
	if (-1 == init_entry_contexts(type, subtype, CFG_HLX_CONTEXTS, idx, entry[CFG_HLX_CONTEXTS])) {
		return -1;
	}

	entry[CFG_HMX_CONTEXTS] = h2_info_unit(unit, CFG_HMX_CONTEXTS);
	if (-1 == init_entry_contexts(type, subtype, CFG_HMX_CONTEXTS, idx, entry[CFG_HMX_CONTEXTS])) {
		return -1;
	}

	return 0;
}

typedef int(*initptr_t)(unsigned int, h2_coproc_type_t, h2_coproc_subtype_t, unsigned int);

static const initptr_t inits[CFG_TYPE_MAX][CFG_SUBTYPE_MAX] = {
	{
		init_entry_vxu0
	}
};

h2_coproc_init_result_t h2_coproc_init() {
	unsigned int unit;
	h2_coproc_type_t type;
	h2_coproc_subtype_t subtype;
	unsigned int idx;

	if (init_done) {
		return oldstyle ? H2_COPROC_INIT_OLDSTYLE : H2_COPROC_INIT_OK;
	}

	if (0 == (unit = h2_info(INFO_UNIT_START))) {  // old style
		oldstyle = 1;
		init_done = 1;
		return H2_COPROC_INIT_OLDSTYLE;
	}

	while (unit) {
		type = h2_info_unit(unit, CFG_UNIT_ID);
		subtype = h2_info_unit(unit, CFG_UNIT_SUBID);
		idx = nunits[type][subtype];
		if (-1 == coproc_grow((void **)&configs[type][subtype], CFG_MAX * (idx + 1) * sizeof(unsigned int))) {
			return H2_COPROC_INIT_ERROR;
		}
		nunits[type][subtype]++;
		if (-1 == inits[type][subtype](unit, type, subtype, idx)) return H2_COPROC_INIT_ERROR;
		unit = h2_info_unit(unit, CFG_UNIT_NEXT);
	}

	init_done = 1;
	return H2_COPROC_INIT_OK;
}
	
