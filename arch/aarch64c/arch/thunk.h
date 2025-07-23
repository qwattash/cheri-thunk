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
#pragma once

#include <stdint.h>

struct thunk_class;

/**
 * Machine-dependent types that represents thunk code buffers.
 */
typedef uint32_t const * thunk_template_t;
typedef uint32_t* thunk_jit_t;

enum aarch64_thunk_reloc_type {
        THUNK_REL_MOV_IMM = 0,
        THUNK_REL_ADR = 1,
        THUNK_REL_LAST
};

/**
 * Thunk template patch point descriptors.
 */
struct aarch64_thunk_reloc {
        enum aarch64_thunk_reloc_type type;
        ptraddr_t addr;
};

typedef struct aarch64_thunk_reloc thunk_reloc_t;

/**
 * Resolved data associated to a patch point descriptor.
 */
union aarch64_thunk_reloc_data {
        uint16_t u16;
        uint32_t u32;
};

typedef union aarch64_thunk_reloc_data thunk_reloc_data_t;

#define THUNK_REL_INITIALIZER(rtype, rpos)  \
        { .type = (THUNK_REL_##rtype), .addr = (rpos) }

/**
 * Internal helper to wrap thunk a capability into a thunk_object_t.
 *
 * Notably, remember to ensure that cap-mode is enabled on the thunk.
 */
static inline void *
thunk_arch_seal_object(uintptr_t obj_ptr)
{
        return ((void *)cheri_sentry_create(obj_ptr | 1));
}

/**
 * Define the maximum permission allowed on the root thunk token.
 */
#define THUNK_TOKEN_MAX_PERMS                             \
        (CHERI_PERM_GLOBAL | CHERI_PERM_STORE_LOCAL_CAP | \
         CHERI_PERM_LOAD | CHERI_PERM_STORE |             \
         CHERI_PERM_LOAD_CAP | CHERI_PERM_STORE_CAP |     \
         CHERI_PERM_MUTABLE_LOAD)
