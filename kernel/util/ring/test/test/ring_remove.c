/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <ringtests.h>
#include <ring.h>
#include <stdio.h>

static void test0()
{
	H2K_ringnode_t *tmp;
	tmp = makering("a");
	H2K_ring_remove(&tmp,getnode('a'));
	checkring(tmp,"");
}

static void test1()
{
	H2K_ringnode_t *tmp;
	tmp = makering("ab");
	H2K_ring_remove(&tmp,getnode('a'));
	checkring(tmp,"b");
}

static void test2()
{
	H2K_ringnode_t *tmp;
	tmp = makering("ba");
	H2K_ring_remove(&tmp,getnode('a'));
	checkring(tmp,"b");
}

static void test3()
{
	H2K_ringnode_t *tmp;
	tmp = makering("abc");
	H2K_ring_remove(&tmp,getnode('a'));
	checkring(tmp,"bc");
}

static void test4()
{
	H2K_ringnode_t *tmp;
	tmp = makering("bac");
	H2K_ring_remove(&tmp,getnode('a'));
	checkring(tmp,"bc");
}

static void test5()
{
	H2K_ringnode_t *tmp;
	tmp = makering("bca");
	H2K_ring_remove(&tmp,getnode('a'));
	checkring(tmp,"bc");
}

void test_ring_remove()
{
	test0();
	test1();
	test2();
	test3();
	test4();
	test5();
}

