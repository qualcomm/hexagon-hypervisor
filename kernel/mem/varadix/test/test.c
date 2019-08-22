/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <context.h>
#include <max.h>
#include <globals.h>
#include <asid.h>
#include <varadix.h>
#include <hexagon_protos.h>

#define TH_printf(...) printf(__VA_ARGS__);  \
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

H2K_thread_context a;

H2K_asid_entry_t info = {
	.ptb = 0,
	.fields = {
		.count = 1,
		.vmid = 2,
		.type = H2K_ASID_TRANS_TYPE_VARADIX,
		.log_maxhops = 2,
		.extra = 0,
	},
};

H2K_asid_entry_t stage2_info = {
	.ptb = 0,
	.fields = {
		.count = 1,
		.vmid = 3,
		.type = H2K_ASID_TRANS_TYPE_OFFSET,
		.log_maxhops = 2,
		.extra = 0,
	},
};

H2K_vmblock_t myvmblock;
H2K_vmblock_t myvmblock2;

int TH_expected_translate_calls = 0;
int TH_translate_idx = 0;
#define MAX_TRANSLATE_CALLS 8
u32_t TH_expected_translate_pn[MAX_TRANSLATE_CALLS];
H2K_asid_entry_t TH_expected_translate_info;

u32_t TH_xwru_mask = ~0;

typedef H2K_translation_t (*translation_funcptr)(H2K_translation_t in, H2K_asid_entry_t info);
translation_funcptr TH_translate_handlers[MAX_TRANSLATE_CALLS];

H2K_translation_t TH_translate_passthrough(H2K_translation_t in, H2K_asid_entry_t info)
{
	// printf("translate: pass: in=%016llx info=%016llx\n",in.raw,info.raw);
	in.xwru &= TH_xwru_mask;
	return in;
}

H2K_translation_t TH_translate_fail(H2K_translation_t in, H2K_asid_entry_t info)
{
	// printf("translate: fail: in=%016llx info=%016llx\n",in.raw,info.raw);
	in.raw = 0;
	return in;
}

H2K_translation_t H2K_translate(H2K_translation_t in, H2K_asid_entry_t info)
{
	if (TH_expected_translate_calls == TH_translate_idx) FAIL("unexpected translate");
	if (info.raw != TH_expected_translate_info.raw) FAIL("translate info wrong");
	if (TH_expected_translate_pn[TH_translate_idx] != in.pn) FAIL("translate pn wrong");
	return TH_translate_handlers[TH_translate_idx++](in,info);
}

static inline u32_t notsorand(u32_t bitsleft)
{
	if (bitsleft == 0) return 0;
	if (bitsleft < 3) return rand() % (bitsleft);
	if ((rand() & 0x7) == 0) return rand() % (bitsleft);
	return rand() & 0x7;
}

#if 0
void check_good(const char *good)
{
	int i;
	u32_t badva;
	H2K_translation_t trans;
	H2K_translation_t check;
	for (i = 0; good[i] != '\0'; i++) {
		badva = (0xa + good[i] - 'a') << 28;
		check.raw = 0;
		check.size = 0;
		check.xwru = 0xf & TH_xwru_mask;
		check.cccc = 0x7;
		check.pn = badva >> 12;
		TH_translate_idx = 0;
		trans = H2K_translate_default(badva);
		trans = H2K_linear_translate(trans,info);
		if (trans.raw != check.raw) {
			printf("%s(%c)\n",good,good[i]);
			printf("%016llx vs %016llx\n",trans.raw,check.raw);
			FAIL("good string failed");
		}
	}
}

void check_bad(const char *bad)
{
	int i;
	u32_t badva;
	H2K_translation_t trans;
	for (i = 0; bad[i] != '\0'; i++) {
		badva = (0xa + bad[i] - 'a') << 28;
		TH_translate_idx = 0;
		trans = H2K_translate_default(badva);
		if ((trans=H2K_linear_translate(trans,info)).raw != 0) {
			printf("%s(%c): %016llx\n",bad,bad[i],trans.raw);
			FAIL("Bad string failed");
		}
	}
}
#endif

typedef struct table_info {
	unsigned long tableaddr;
	struct table_info *next;
	u32_t size;
	void *storage;
} TH_table_info;

static inline u32_t entry_size(u32_t entry)
{
	return Q6_R_ct0_R(entry);
}

static inline int should_be_terminal(u32_t size,u32_t bitsleft)
{
	if (size > bitsleft) return 1;
	if (size < 3) return rand() & 1;
	return ((rand() % (size)) == 0);
}

TH_table_info *alloc_random_tables(TH_table_info *next, u32_t bitsleft, u32_t bitsdone)
{
	u32_t size;
	unsigned long tableaddr;
	void *table_storage;
	TH_table_info *newtab;
	size = notsorand(bitsleft)+1;
	if (size >= bitsleft) {
		if (should_be_terminal(size,bitsleft)) {
			return next;
		}
	}
	if ((table_storage = calloc(1,8<<size)) == NULL) FAIL("calloc!");
	if ((newtab = calloc(1,sizeof(TH_table_info))) == NULL) FAIL("calloc!");
	newtab->size = size;
	newtab->storage = table_storage;
	newtab->next = next;
	/* Round up table address */
	tableaddr = (long)table_storage;
	tableaddr += ((4<<size)-1);
	tableaddr &= (-4 << size);
	newtab->tableaddr = tableaddr;
	return alloc_random_tables(newtab,bitsleft-size,bitsdone+size);
}

void free_tables(TH_table_info *root)
{
	TH_table_info *next;
	if (root == NULL) return;
	next = root->next;
	free(root->storage);
	free(root);
	free_tables(next);
}

static inline u32_t make_terminal_entry(u32_t bitsleft)
{
	u32_t xwru;
	u32_t cccc;
	u32_t pn;
	while ((xwru = rand() & 0xF) < 2) /* RETRY */;
	cccc = rand() & 0xF;
	pn = rand() & 0x00FFFFFF;
	pn |= cccc << 24;
	pn |= xwru << 28;
	pn = Q6_R_insert_RP(pn,1<<bitsleft,Q6_P_combine_RR(bitsleft+1,0));
	return pn;
}

static inline u32_t make_table_entry(TH_table_info *table)
{
	u32_t tableaddr = table->tableaddr;
	tableaddr >>= 2;
	tableaddr = Q6_R_insert_RP(tableaddr,1<<(table->size-1),Q6_P_combine_RR((table->size),0));
	return tableaddr;
}

u32_t gen_random_tables(TH_table_info *table, u32_t bitsleft, u32_t bitsdone, u32_t *good_vpn)
{
	u32_t *table_base;
	u32_t *entry_ptr;
	u32_t idx;
	u32_t startbit;
	if (table == NULL) {
		/* TERMINAL! */
		return make_terminal_entry(bitsleft);
	} else {
		/* POINTER! */
		startbit = bitsleft-(table->size);
		table_base = (u32_t *)table->tableaddr;
		idx = rand() % (1<<table->size);
		entry_ptr = table_base + idx;
		*entry_ptr = gen_random_tables(table->next,bitsleft-table->size,bitsdone+table->size,good_vpn);
		*good_vpn |= idx << startbit;
		return make_table_entry(table);
	}
}

u32_t get_terminal_entry(TH_table_info *table, u32_t bitsleft, u32_t vpn, u32_t last_entry)
{
	u32_t *table_base;
	u32_t startbit;
	u32_t idx;
	if (table == NULL) return last_entry;
	table_base = (u32_t *)table->tableaddr;
	startbit = bitsleft-table->size;
	idx = (vpn >> startbit) & ((1<<table->size)-1);
	last_entry = table_base[idx];
	return get_terminal_entry(table->next,bitsleft-table->size,vpn,last_entry);
}

void pprint_table(TH_table_info *table, u32_t bitsleft, u32_t goodvpn, char *prefix)
{
	u32_t size;
	u32_t startbit;
	u32_t index;
	if (table == NULL) {
		TH_printf("%sterminal: bitsleft=%x goodvpn=%x\n",prefix,bitsleft,goodvpn);
		return;
	}
	size = table->size;
	startbit = bitsleft - size;
	index = (goodvpn >> startbit) & ((1<<size)-1);
	TH_printf("%sbitsleft=%2u: table @ %08lx size %2u startbit %2u goodvpn=%08x index=%2d\n",
		prefix,
		bitsleft,
		table->tableaddr,
		size,
		startbit,
		goodvpn,
		index);
	pprint_table(table->next,startbit,goodvpn,"...");
}

void setup_machine(u32_t root_entry, u32_t vpn_bits)
{
	info.ptb = root_entry;
	info.fields.extra = vpn_bits;
}

void test_good(u32_t good_vpn,u32_t vpn_bits, u32_t terminal_entry)
{
	u32_t terminal_size = entry_size(terminal_entry);
	H2K_translation_t trans;
	H2K_translation_t expected_trans;
	expected_trans.xwru = terminal_entry >> 28;
	expected_trans.cccc = terminal_entry >> 24;
	expected_trans.pn = ((terminal_entry & 0x007ffffe) >> 1) & (-1 << terminal_size);
	int i;
	for (i = 0; i < terminal_size; i++) {
		trans.pn = good_vpn + i;
		trans.xwru = 0xf;
		trans.cccc = 0xff;
		trans = H2K_varadix_translate(trans,info);
		if (trans.raw == 0) {
			TH_printf("trans: %08x --> %016llx\n",good_vpn + i,trans.raw);
			FAIL("good walk failed");
		}
		if (trans.xwru != expected_trans.xwru) {
			TH_printf("trans xwru: %x expected xwru: %x terminal_entry=%x\n",trans.xwru, expected_trans.xwru,terminal_entry);
			FAIL("bad xwru");
		}
		if (trans.cccc != expected_trans.cccc) FAIL("bad cccc");
		if (trans.pn != (expected_trans.pn | i)) {
			TH_printf("trans.pn = %08x expected_trans.pn=%08x i=%08x\n",
				trans.pn,
				expected_trans.pn,
				i);
			FAIL("bad pn");
		}
	}
}

void test_bad(u32_t good_vpn,u32_t vpn_bits, u32_t terminal_entry)
{
	u32_t terminal_size = entry_size(terminal_entry);
	u32_t startbit = terminal_size;
	H2K_translation_t trans;
	trans.xwru = 0xf;
	trans.cccc = 0x7;
	int i;
	for (i = startbit; i < vpn_bits; i++) {
		trans.pn = good_vpn ^ (1<<i);
		// TH_printf("i=%d good_vpn=%x bad_vpn = %x\n",i,good_vpn,trans.pn);
		trans = H2K_varadix_translate(trans,info);
		if (trans.raw) {
			TH_printf("trans = %016llx\n",trans.raw);
			FAIL("bad walk passed");
		}
	}
	trans.pn = 1<<vpn_bits;
	trans = H2K_varadix_translate(trans,info);
	if (trans.raw) FAIL("bad walk passed (> va space)");
}

void TH_random_test()
{
	TH_table_info *tables;
	u32_t vpn_bits = rand() % 23;
	u32_t root_entry;
	u32_t terminal_entry;
	u32_t goodvpn = 0;
	tables = alloc_random_tables(NULL,vpn_bits,0);
	root_entry = gen_random_tables(tables,vpn_bits,0,&goodvpn);
	terminal_entry = get_terminal_entry(tables,vpn_bits,goodvpn,root_entry);
	setup_machine(root_entry,vpn_bits);
	// pprint_table(tables,vpn_bits,goodvpn,"");
	test_good(goodvpn,vpn_bits,terminal_entry);
	test_bad(goodvpn,vpn_bits,terminal_entry);
	free_tables(tables);
}

int main()
{
	int i;
	__asm__ __volatile(GLOBAL_REG_STR " = %0 " : : "r"(&H2K_kg));
	srand(42);
	H2K_gp->vmblocks[2] = &myvmblock;
	H2K_gp->vmblocks[3] = &myvmblock2;
	myvmblock.guestmap.raw = 0;
	TH_expected_translate_calls = 0;

	for (i = 0; i < 10000; i++) {
		TH_random_test();
		// TH_printf("test %d complete!\n",i);
	}

	puts("TEST PASSED");
	return 0;
}

