/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <h2_coproc.h>
#include <h2_alloc.h>

static unsigned int *configs[CFG_TYPE_MAX][CFG_SUBTYPE_MAX] = {NULL};
static unsigned int nunits[CFG_TYPE_MAX][CFG_SUBTYPE_MAX] = {0};
static unsigned int *counts[CFG_TYPE_MAX][CFG_SUBTYPE_MAX][CFG_MAX] = {NULL};
static unsigned int oldstyle = 0;
static unsigned int init_done = 0;

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
	unsigned int bits;
	unsigned int xa;
	unsigned int *entry;

	if (oldstyle || !enable) {
		return setbits(type, subtype, entry_type, num, enable);  // 
	}

	num += 1;  // num == number of existing contexts left to find
	
	while (num && idx < nunits[type][subtype]) {  // still searching and in range
		if (unit_mask & (0x1 << idx)) {  // unit selected
			entry = &configs[type][subtype][idx * CFG_MAX];
			bits = entry[entry_type];
			ncontexts = Q6_R_popcount_P(bits);
			if (num <= ncontexts) {  // requested context # is in this unit
				xa = 0;  // index of bit
				do {
					if (bits & 0x1) {  // context exists
						if (0 == (--num)) {  // none left to find; now xa is correct
							break;
						} else {
							xa++;
							continue;
						}
					} else {  // context is missing
						bits >>= 1;  // check next bit
						xa++;
						continue;
					}
				} while (1);

				// now xa is the index of the bit in the current unit that corresponds to the requested context #
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

static int init_entry_vxu0(unsigned int unit, h2_coproc_type_t type, h2_coproc_subtype_t subtype, unsigned int idx) {
	unsigned int *entry;

	entry = &configs[type][subtype][idx * CFG_MAX];
	entry[CFG_VXU_UNIT_ID] = h2_info_unit(unit, CFG_VXU_UNIT_ID);

	entry[CFG_HVX_CONTEXTS] = h2_info_unit(unit, CFG_HVX_CONTEXTS);
	if (NULL == (counts[type][subtype][CFG_HVX_CONTEXTS] =
							 h2_realloc(counts[type][subtype][CFG_HVX_CONTEXTS], (idx + 1) * sizeof(unsigned int)))) {
		return -1;
	}
	counts[type][subtype][CFG_HVX_CONTEXTS][idx] = Q6_R_popcount_P(entry[CFG_HVX_CONTEXTS]);
		
	entry[CFG_HLX_CONTEXTS] = h2_info_unit(unit, CFG_HLX_CONTEXTS);
	if (NULL == (counts[type][subtype][CFG_HLX_CONTEXTS] =
							 h2_realloc(counts[type][subtype][CFG_HLX_CONTEXTS], (idx + 1) * sizeof(unsigned int)))) {
		return -1;
	}
	counts[type][subtype][CFG_HLX_CONTEXTS][idx] = Q6_R_popcount_P(entry[CFG_HLX_CONTEXTS]);
		
	entry[CFG_HMX_CONTEXTS] = h2_info_unit(unit, CFG_HMX_CONTEXTS);
	if (NULL == (counts[type][subtype][CFG_HMX_CONTEXTS] =
							 h2_realloc(counts[type][subtype][CFG_HMX_CONTEXTS], (idx + 1) * sizeof(unsigned int)))) {
		return -1;
	}
	counts[type][subtype][CFG_HMX_CONTEXTS][idx] = Q6_R_popcount_P(entry[CFG_HMX_CONTEXTS]);
		
	return 0;
}

typedef int(*initptr_t)(unsigned int, h2_coproc_type_t, h2_coproc_subtype_t, unsigned int);

static const initptr_t inits[CFG_TYPE_MAX][CFG_SUBTYPE_MAX] = {
	{
		init_entry_vxu0
	}
};

int h2_coproc_init() {
	unsigned int unit;
	h2_coproc_type_t type;
	h2_coproc_subtype_t subtype;
	unsigned int idx;

	if (init_done) {
		return 0;
	}
		
	if (0 == (unit = h2_info(INFO_UNIT_START))) {  // old style
		oldstyle = 1;
		init_done = 1;
		return 1;
	}

	init_done = 1;
	while (unit) {
		type = h2_info_unit(unit, CFG_UNIT_ID);
		subtype = h2_info_unit(unit, CFG_UNIT_SUBID);
		idx = nunits[type][subtype];
		if (NULL == (configs[type][subtype] =
								 h2_realloc(configs[type][subtype], CFG_MAX * (idx + 1) * sizeof(unsigned int)))) {
			return -1;
		}
		nunits[type][subtype]++;
		if (-1 == inits[type][subtype](unit, type, subtype, idx)) return -1;
		unit = h2_info_unit(unit, CFG_UNIT_NEXT);
	}

	/* printf ("nunits 0x%08x\n", nunits[0][0]); */

	/* printf ("unit ID 0x%08x\n", configs[0][0][CFG_VXU_UNIT_ID]); */

	/* printf ("configs 0x%08x\n", configs[0][0][CFG_HVX_CONTEXTS]); */
	/* printf ("configs 0x%08x\n", configs[0][0][CFG_HLX_CONTEXTS]); */
	/* printf ("configs 0x%08x\n", configs[0][0][CFG_HMX_CONTEXTS]); */

	/* printf ("counts 0x%08x\n", counts[0][0][CFG_HVX_CONTEXTS][0]); */
	/* printf ("counts 0x%08x\n", counts[0][0][CFG_HLX_CONTEXTS][0]); */
	/* printf ("counts 0x%08x\n", counts[0][0][CFG_HMX_CONTEXTS][0]); */

	/* printf ("unit ID 0x%08x\n", configs[0][0][CFG_MAX + CFG_VXU_UNIT_ID]); */

	/* printf ("configs 0x%08x\n", configs[0][0][CFG_MAX + CFG_HVX_CONTEXTS]); */
	/* printf ("configs 0x%08x\n", configs[0][0][CFG_MAX + CFG_HLX_CONTEXTS]); */
	/* printf ("configs 0x%08x\n", configs[0][0][CFG_MAX + CFG_HMX_CONTEXTS]); */

	/* printf ("counts 0x%08x\n", counts[0][0][CFG_HVX_CONTEXTS][1]); */
	/* printf ("counts 0x%08x\n", counts[0][0][CFG_HLX_CONTEXTS][1]); */
	/* printf ("counts 0x%08x\n", counts[0][0][CFG_HMX_CONTEXTS][1]); */

	return 0;
}
	
