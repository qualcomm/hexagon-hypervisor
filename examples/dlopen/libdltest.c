/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <stdlib.h>
#include <dlfcn.h>
#include <stdio.h>

int main(int argc, char *argv[])
{
	const char *(*fptr)();
	const char *(*cfunc)();

	const char *builtin[] = {"libgcc.so", "libc.so", "libstdc++.so",
	                         "libc++.so.1", "libc++abi.so.1"};
	dlinit(5, (char **)builtin);

	/* Test C shared library */
	void *dl = dlopen("french.so", RTLD_NOW);
	if (dl == NULL) {
		printf("french.so failed: %s\n", dlerror());
		return -1;
	}
	*(void **)(&fptr) = dlsym(dl, "faux");
	if (fptr == NULL) {
		printf("faux not found\n");
		return -1;
	}
	printf("faux @ %p\n", (void *)fptr);
	const char *mystr = fptr();
	printf("faux returned: %s\n", mystr);

	/* Test C++ shared library */
	void *cppso = dlopen("cppso.so", RTLD_NOW);
	if (cppso == NULL) {
		printf("cppso.so failed: %s\n", dlerror());
	} else {
		*(void **)(&cfunc) = dlsym(cppso, "cppfunc");
		if (cfunc == NULL)
			printf("cppfunc not found\n");
		else
			printf("cppfunc returned: %s\n", cfunc());
	}

	return 0;
}
