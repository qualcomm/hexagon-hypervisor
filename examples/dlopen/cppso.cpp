/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <stdio.h>

struct SoCtor {
    SoCtor()  { printf("C++ ctor: cppso.so\n"); }
    ~SoCtor() { printf("C++ dtor: cppso.so\n"); }
};

static SoCtor so_ctor;

extern "C" const char *cppfunc()
{
    return "Hello from C++ shared library!";
}
