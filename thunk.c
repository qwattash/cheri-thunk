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
        struct thunk_metaclass *mc = tc->mc;
        thunk_object_t obj = THUNK_NULLOBJ;
        uintptr_t thunk_buf;
        uintptr_t obj_data;
        thunk_jit_t obj_code;

        // XXX tc should be sealed and should be authorised here

        assert(tc->object_size > mc->code_size &&
            "Invalid thunk class, code size > object size");
        /* object_size must already include any representability padding */
        thunk_buf = (uintptr_t)thunk_xmalloc(tc->object_size);

        obj_code = (thunk_jit_t)cheri_bounds_set(thunk_buf, tc->mc->code_size);
        obj_data = thunk_buf + cheri_representable_length(tc->mc->code_size);
        obj_data = cheri_bounds_set_exact(obj_data,
            thunk_buf + cheri_length_get(thunk_buf) - obj_data);
        if (thunk_compile(obj_code, tc)) {
                thunk_xfree((void *)thunk_buf);
                goto out;
        }

        if (tc->ctor)
                tc->ctor((void *)obj_data);

        obj = thunk_object_wrap(thunk_arch_seal_object(thunk_buf));
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
