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

#include <assert.h>
#include <cheriintrin.h>
#include <stdio.h>
#include <string.h>

#include "hello/hello.h"

#define DATA_PERMS_MASK                                                 \
        (CHERI_PERM_EXECUTE | CHERI_PERM_STORE | CHERI_PERM_STORE_CAP)

int
main(int argc, char *argv[])
{
        hello_object_t h = hello_create();

        assert(cheri_is_sealed(thunk_object_unwrap(h)) && "Thunk is unsealed");

        const char *data = hello_invoke(h);

        assert(cheri_is_valid(data) && "Invalid thunk result");
        assert((cheri_perms_get(data) & DATA_PERMS_MASK) == 0 &&
            "Thunk enforced wrong permission");
        assert(strcmp(data, "Hello World!") == 0 && "Invalid thunk data");

        return (0);
}
