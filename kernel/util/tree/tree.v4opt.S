/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <asm_std.h>
#include <asm_offsets.h>

#define LEFT_OFF 0
#define RIGHT_OFF 4
#define KEY_OFF 8

// void H2K_tree_remove_key(H2K_treenode_t **root, H2K_treenode_t *node, H2K_treekey_t key)
#define ROOT r0
#define NODE r1
#define KEY r3:2
#define RIGHT r5
#define LEFT r4
#define RIGHTLEFT r5:4
#define ROOTNODE r6
#define ROOTKEY r9:8
FUNC_START H2K_tree_remove_key  .text.util.tree  .p2align  3
	{
		ROOTNODE = memw(ROOT)
		RIGHTLEFT = memd(NODE)
	}
	{
		p0 = cmp.eq(ROOTNODE,#0)
		if (p0.new) jump:nt 9f
		p1 = cmp.eq(NODE,ROOTNODE)
		if (p1.new) jump:nt 2f
		if (!p0.new) ROOTKEY = memd(ROOTNODE+#KEY_OFF)
	}
	{
		p2 = cmp.gt(KEY,ROOTKEY)
		if (!p2.new) ROOT = add(ROOTNODE,#LEFT_OFF)
		if (p2.new) ROOT = add(ROOTNODE,#RIGHT_OFF)
		jump H2K_tree_remove_key
	}
2:
	{
		memw(ROOT) = LEFT
		p1 = cmp.eq(RIGHT,#0)
		if (!p1.new) jump:t H2K_tree_add_key
		NODE = RIGHT
		if (!p1.new) KEY = memd(RIGHT+#KEY_OFF)
	}
9:
	{
		jumpr r31
	}
FUNC_END H2K_tree_remove_key

FUNC_START H2K_tree_add_key  .text.util.tree  .falign
	ROOTNODE = memw(ROOT)
	{
		p0 = cmp.eq(ROOTNODE,#0)
		if (p0.new) memw(ROOT) = NODE
		if (p0.new) jumpr:nt r31
		if (!p0.new) ROOTKEY = memd(ROOTNODE+#KEY_OFF)
	}
	{
		p2 = cmp.gt(KEY,ROOTKEY)
		if (!p2.new) ROOT=add(ROOTNODE,#LEFT_OFF)
		if (p2.new) ROOT=add(ROOTNODE,#RIGHT_OFF)
		jump H2K_tree_add_key
	}
FUNC_END H2K_tree_add_key
#undef ROOT
#undef NODE
#undef KEY
#undef RIGHT
#undef LEFT
#undef RIGHTLEFT
#undef ROOTNODE
#undef ROOTKEY

FUNC_START H2K_tree_bisect  .text.util.tree  .falign
#define LE_TREE_P r0
#define GT_TREE_P r1
#define GTLE_TREE_P r1:0
#define ROOT r2
#define TMP r3
#define KEY r5:4
#define ROOTKEY r7:6
#define RIGHT r9
#define LEFT r8
#define RIGHTLEFT r9:8
#define NEWGT r11
#define NEWLE r10
#define NEWGTLE r11:10

	{
		p0 = cmp.eq(ROOT,#0)
		if (p0.new) jumpr:nt r31
		if (!p0.new) ROOTKEY = memd(ROOT+#KEY_OFF)
		if (!p0.new) RIGHTLEFT = memd(ROOT+#LEFT_OFF)
	}
	{
		p1 = cmp.gt(ROOTKEY,KEY)
		if (!p1.new) NEWGTLE = combine(RIGHT,ROOT)
		if (p1.new) NEWGTLE = combine(ROOT,LEFT)
	}
	{
		memw(LE_TREE_P) = NEWLE
		memw(GT_TREE_P) = NEWGT
		if (!p1) LE_TREE_P = add(ROOT,#RIGHT_OFF)
		if (p1) GT_TREE_P = add(ROOT,#LEFT_OFF)
	}
	{
		if (!p1) memw(ROOT+#RIGHT_OFF) = #0
		if (p1) memw(ROOT+#LEFT_OFF) = #0
		ROOT = mux(p1,LEFT,RIGHT)
		jump H2K_tree_bisect
	}

#undef LE_TREE_P
#undef GT_TREE_P
#undef GTLE_TREE_P
#undef ROOT
#undef TMP
#undef KEY
#undef ROOTKEY
#undef RIGHT
#undef LEFT
#undef RIGHTLEFT
#undef NEWGT
#undef NEWLE
#undef NEWGTLE
FUNC_END H2K_tree_bisect

FUNC_START H2K_tree_destructive_iterate  .text.util.tree  .falign
#define ROOT r0
#define OPAQUE r1
#define OPAQUE_ROOT r1:0
#define FUNC r2
#define RIGHT r5
#define LEFT r4
#define RIGHTLEFT r5:4
#define RIGHT2 r7
#define LEFT2 r6
#define RIGHTLEFT2 r7:6
#define OPAQUE_SAV r17
#define ROOT_SAV r16
#define OPAQUE_ROOT_SAV r17:16
#define FUNC_SAV r18

	{
		allocframe(#16)
		p0 = cmp.eq(ROOT,#0)
		if (p0.new) jump:nt 9f
		if (!p0.new) RIGHTLEFT = memd(ROOT)
	}
	{
		memd(r29+#0) = OPAQUE_ROOT_SAV
		memw(r29+#8) = FUNC_SAV
		OPAQUE_ROOT_SAV = combine(OPAQUE,ROOT)
		FUNC_SAV = FUNC
	}
	.falign
1:
	{
		p0 = cmp.eq(LEFT,#0)
		if (!p0.new) RIGHTLEFT2 = memd(LEFT)
	}
	{
		if (!p0) memw(ROOT+#LEFT_OFF) = RIGHT2
		if (!p0) memw(LEFT+#RIGHT_OFF) = ROOT
		if (p0) callr FUNC_SAV
		ROOT_SAV = mux(p0,RIGHT,LEFT)
	}
	{
		p0 = cmp.eq(ROOT_SAV,#0)
		if (!p0.new) RIGHTLEFT = memd(ROOT_SAV)
		if (!p0.new) jump:t 1b
		OPAQUE_ROOT = OPAQUE_ROOT_SAV
	}
	{
		OPAQUE_ROOT_SAV = memd(r29+#0)
		FUNC_SAV = memw(r29+#8)
	}
9:
	dealloc_return
#undef ROOT
#undef OPAQUE
#undef OPAQUE_ROOT
#undef FUNC
#undef RIGHT
#undef LEFT
#undef RIGHTLEFT
#undef RIGHT2
#undef LEFT2
#undef RIGHTLEFT2
#undef OPAQUE_SAV
#undef ROOT_SAV
#undef OPAQUE_ROOT_SAV
#undef FUNC_SAV
FUNC_END H2K_tree_destructive_iterate

FUNC_START H2K_tree_find  .text.util.tree  .falign
#define ROOT r0
#define NEWROOT r1
#define KEY r3:2
#define ROOTKEY r5:4
#define RIGHT r7
#define LEFT r6
#define RIGHTLEFT r7:6
	{
		p0 = cmp.eq(ROOT,#0)
		if (p0.new) jump:nt 9f
		if (!p0.new) ROOTKEY = memd(ROOT+#KEY_OFF)
		if (!p0.new) RIGHTLEFT = memd(ROOT+#LEFT_OFF)
	}
	{
		p0 = cmp.eq(ROOTKEY,KEY)
		p1 = cmp.gt(ROOTKEY,KEY)
		if (p1.new) NEWROOT = LEFT
		if (!p1.new) NEWROOT = RIGHT
	}
	{
		if (!p0) jump H2K_tree_find
		if (!p0) ROOT = NEWROOT
	}
9:
	jumpr r31
#undef ROOT
#undef KEY
#undef ROOTKEY
#undef RIGHTLEFT
FUNC_END H2K_tree_find

