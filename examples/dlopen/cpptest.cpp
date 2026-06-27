/*
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause-Clear
 */
#include <stdio.h>

struct MainCtor {
    MainCtor()  { printf("C++ ctor: main binary\n"); }
    ~MainCtor() { printf("C++ dtor: main binary\n"); }
};

static MainCtor main_ctor;
