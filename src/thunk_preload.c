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

/*
 * When using software-defined permission bits to authenticate thunks,
 * we need to hook the system malloc to strip the permission bit from
 * normal allocations.
 *
 * This file compiles to a separate shared object that should be LD_PRELOAD'ed.
 *
 * XXX thunk_xmalloc should be allowed through, although there is
 * an argument for a custom allocator there anyway.
 */
#include <cheriintrin.h>
#include <dlfcn.h>
#include <stdlib.h>

#include "arch/thunk.h"

typedef void *(*malloc_fn_t)(size_t);
typedef void *(*calloc_fn_t)(size_t, size_t);
typedef void *(*realloc_fn_t)(void *, size_t);
typedef void (*free_fn_t)(void *);

static malloc_fn_t system_malloc;
static calloc_fn_t system_calloc;
static realloc_fn_t system_realloc;
static free_fn_t system_free;

__attribute__((constructor))
static void
thunk_preload_ctor()
{
        system_malloc = (malloc_fn_t)dlfunc(RTLD_NEXT, "malloc");
        system_free = (free_fn_t)dlfunc(RTLD_NEXT, "free");
        system_realloc = (realloc_fn_t)dlfunc(RTLD_NEXT, "realloc");
        system_calloc = (calloc_fn_t)dlfunc(RTLD_NEXT, "calloc");
}

#ifdef THUNK_AUTH_MODE_PERMS
void *
malloc(size_t size)
{
        void *ptr = system_malloc(size);

        return (cheri_perms_clear(ptr, CHERI_PERM_SW_THUNK));
}

void *
calloc(size_t memb, size_t size)
{
        void *ptr = system_calloc(memb, size);

        return (cheri_perms_clear(ptr, CHERI_PERM_SW_THUNK));
}

void *
realloc(void *ptr, size_t size)
{
        ptr = system_realloc(ptr, size);

        return (cheri_perms_clear(ptr, CHERI_PERM_SW_THUNK));
}

void
free(void *ptr)
{
        system_free(ptr);
}

#endif
