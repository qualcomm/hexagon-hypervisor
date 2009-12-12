/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <ringtests.h>
#include <ring.h>
#include <stdio.h>

static void test0()
{
	H2K_ringnode_t *src, *dst;
	dst = makering("");
	src = makering("a");
	H2K_ring_remove_append(&src,&dst,getnode('a'));
	checkring(src,"");
	checkring(dst,"a");
}

static void test1()
{
	H2K_ringnode_t *src, *dst;
	dst = makering("");
	src = makering("ab");
	H2K_ring_remove_append(&src,&dst,getnode('a'));
	checkring(src,"b");
	checkring(dst,"a");
}

static void test2()
{
	H2K_ringnode_t *src, *dst;
	dst = makering("");
	src = makering("ba");
	H2K_ring_remove_append(&src,&dst,getnode('a'));
	checkring(src,"b");
	checkring(dst,"a");
}

static void test3()
{
	H2K_ringnode_t *src, *dst;
	dst = makering("c");
	src = makering("a");
	H2K_ring_remove_append(&src,&dst,getnode('a'));
	checkring(src,"");
	checkring(dst,"ca");
}

static void test4()
{
	H2K_ringnode_t *src, *dst;
	dst = makering("c");
	src = makering("ab");
	H2K_ring_remove_append(&src,&dst,getnode('a'));
	checkring(src,"b");
	checkring(dst,"ca");
}

static void test5()
{
	H2K_ringnode_t *src, *dst;
	dst = makering("c");
	src = makering("ba");
	H2K_ring_remove_append(&src,&dst,getnode('a'));
	checkring(src,"b");
	checkring(dst,"ca");
}

void test_ring_remove_append()
{
	test0();
	test1();
	test2();
	test3();
	test4();
	test5();
}

