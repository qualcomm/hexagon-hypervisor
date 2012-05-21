/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */

#include <c_std.h>
#include <stdio.h>
#include <stdlib.h>
#include <thread.h>
#include <tree.h>

void FAIL(const char *str)
{
	puts("FAIL");
	puts(str);
	exit(1);
}

typedef struct {
	const char *name;
	unsigned int size;
	unsigned int min;
	unsigned int max;
	unsigned int *data;
} test_t;

#define TEST(NAME,SIZE,MIN,MAX,...) static unsigned int _testdata_##NAME [] = __VA_ARGS__ ;
#include "tests.dat"
#undef TEST

test_t tests[] = {
#define TEST(NAME,SIZE,MIN,MAX,...) { #NAME, SIZE, MIN, MAX, _testdata_##NAME },
#include "tests.dat"
#undef TEST
	{ NULL, 0, 0, 0, NULL}
};

void treetest(H2K_treenode_t *root, unsigned int min, unsigned int max)
{
	if (root == NULL) return;
	if ((root->key) < min) FAIL("Node too small");
	if ((root->key) > max) FAIL("Node too big");
	treetest(root->left,min,root->key);
	treetest(root->right,root->key,max);
}

static unsigned int *testdata;
static unsigned int testidx;
static unsigned int testidxmax;

static int testdatacmp(unsigned int *x, unsigned int *y)
{
	if (*x == *y) return 0;
	if (*x < *y) return -1;
	return 1;
}

void init_teardown(test_t *test)
{
	int i;
	if ((testdata = malloc(sizeof(*testdata)*test->size)) == NULL) FAIL("malloc");
	for (i = 0; i < test->size; i++) {
		testdata[i] = test->data[i];
	}
	qsort(testdata,test->size,sizeof(testdata[0]),(void *)testdatacmp);
	testidx = 0;
	testidxmax = test->size;
}

void teardown_check(H2K_treenode_t *root, void *ptr)
{
	if (testidx >= testidxmax) FAIL("Too many calls to teardown");
	if (root->key != testdata[testidx++]) FAIL("Unexpected teardown order");
	if (ptr != NULL) FAIL("Unexpected opaque ptr");
	free(root);
}

void teardown_finalize()
{
	if (testidx != testidxmax) FAIL("Didn't call teardown for each element");
}

/*
 * For each test:
 *   * Build up the tree
 *   * Verify the tree is OK
 *   * 10 times:
 *      * Remove the head, verify tree
 *      * Add the former head back into the tree, verify tree
 *   * Bisect the tree 
 *   * Destructive Iterate the tree, freeing nodes
 */
int main()
{
	int idx,i;
	test_t *test;
	H2K_treenode_t *root = NULL;
	H2K_treenode_t *tmp;
	H2K_treenode_t *leftroot;
	H2K_treenode_t *rightroot;
	unsigned int keytmp;
	for (idx = 0; tests[idx].name != NULL; idx++) {
		printf("%s: %d elements [%d,%d]\n",tests[idx].name,tests[idx].size,tests[idx].min,tests[idx].max);
		test = &(tests[idx]);
		root = NULL;
		for (i = 0; i < test->size; i++) {
			if ((tmp = malloc(sizeof(*tmp))) == NULL) FAIL("malloc");
			tmp->rightleft = 0;
			tmp->key = test->data[i];
			//printf("Adding %x key=%d\n",tmp,test->data[i]);
			H2K_tree_add(&root,tmp);
			if ((i & 0x1f) == 0) treetest(root,test->min,test->max);
		}
		treetest(root,test->min,test->max);

                /* look for a big number we know is not in the list (max 700?) */
		tmp = H2K_tree_find(root,777);
		if (tmp) FAIL("Found a node that doesn't exist!");

		/* Find, Remove, then Re-Add 10 random nodes */
		for (i = 0; i < 10; i++) {
			keytmp = test->data[rand() % test->size];
			tmp = H2K_tree_find(root,keytmp);
			if (tmp == 0) FAIL("Couldn't find node I know is there!");
			H2K_tree_remove(&root,tmp);
			tmp->rightleft = 0;
			treetest(root,test->min,test->max);
			H2K_tree_add(&root,tmp);
			treetest(root,test->min,test->max);
		}

		leftroot = NULL;
		rightroot = NULL;
		keytmp = root->key;
		H2K_tree_bisect(&leftroot,&rightroot,root,root->key);
		treetest(leftroot,test->min,keytmp);
		treetest(rightroot,keytmp,test->max);
		init_teardown(test);
		H2K_tree_destructive_iterate(leftroot,NULL,teardown_check);
		H2K_tree_destructive_iterate(rightroot,NULL,teardown_check);
		teardown_finalize();
	}
	puts("TEST PASSED");
	return 0;
}

