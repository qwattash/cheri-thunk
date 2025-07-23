/*-
 * Copyright (c) 2025 Alfredo Mazzinghi
 *
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * This software was developed by SRI International, the University of
 * Cambridge Computer Laboratory (Department of Computer Science and
 * Technology), and Capabilities Limited under Defense Advanced Research
 * Projects Agency (DARPA) Contract No. FA8750-24-C-B047 ("DEC").
 */

#include "thunk.h"

/**
 * Sealed thunk object.
 */
typedef union {
        thunk_object_t _o;
        const char* (*_invoke)(void);
} hello_object_t;

hello_object_t hello_create();
void hello_destroy(hello_object_t obj);

static inline const char *
hello_invoke(hello_object_t obj)
{
        return (obj._invoke());
}

