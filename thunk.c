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
#include <stdlib.h>

#include <machine/param.h>

#include "thunk.h"

/**
 * Executable memory allocation hook
 */
__attribute__((weak))
void *
thunk_xmalloc(size_t size)
{
        fprintf(stderr, "Missing thunk_xmalloc\n");
        abort();
}

/**
 * Executable memory free hook
 */
__attribute__((weak))
void
thunk_xfree(void *ptr)
{
        fprintf(stderr, "Missing thunk_xfree\n");
        abort();
}

thunk_object_t
thunk_malloc(struct thunk_class *tc)
{
        thunk_object_t obj = { .__inner = NULL };
        char *thunk_buf;
        thunk_jit_t obj_code;
        char *obj_data;

        // XXX tc should be sealed and should be authorised here

        /* object_size must already include any representability padding */
        thunk_buf = thunk_xmalloc(tc->object_size);

        obj_code = (thunk_jit_t)cheri_bounds_set(thunk_buf, tc->mc->code_size);
        obj_data = thunk_buf + cheri_representable_length(tc->mc->code_size);
        assert(obj_data - thunk_buf == tc->object_size &&
            "Invalid thunk object size");
        if (thunk_compile(obj_code, tc)) {
                thunk_xfree(thunk_buf);
                goto out;
        }

        // Run thunk constructor

        obj.__inner = cheri_sentry_create(thunk_buf);
out:
        return (obj);
}


void
thunk_free(struct thunk_class *tc, thunk_object_t obj)
{
        thunk_xfree(obj.__inner);
}

void *
thunk_level_malloc(size_t size, thunk_level_t level)
{
        void *mem;

        mem = malloc(size);
        if (mem == NULL)
                return (mem);

        if (level == THUNK_LEVEL_PRIVATE) {
                mem = cheri_perms_clear(mem, CHERI_PERM_GLOBAL);
        } else if (level == THUNK_LEVEL_SHAREABLE) {
                mem = cheri_perms_clear(mem, CHERI_PERM_STORE_LOCAL_CAP);
        }

        return (mem);
}
