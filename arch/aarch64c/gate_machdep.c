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

#include <cheriintrin.h>
#include <stddef.h>
#include <stdlib.h>

#include "thunk-gate.h"
#include "arch/thunk-patch.h"

THUNK_DECL_TEMPLATE(gate);
THUNK_DECL_PATCH_POINT(gate, token_base_0);
THUNK_DECL_PATCH_POINT(gate, token_base_16);
THUNK_DECL_PATCH_POINT(gate, token_base_32);
#ifdef THUNK_LARGE_TOKEN_SPACE
THUNK_DECL_PATCH_POINT(gate, token_base_48);
#endif
THUNK_DECL_PATCH_POINT(gate, data_offset);

#ifdef THUNK_LARGE_TOKEN_SPACE
#define THUNK_GATE_NRELOCS 5
#else
#define THUNK_GATE_NRELOCS 4
#endif

/**
 * Thunk gate metaclass, uses C-style structure inheritance.
 */
struct thunk_gate_metaclass {
        THUNK_METACLASS_HEADER;
        thunk_reloc_t relocs[THUNK_GATE_NRELOCS];
};

static_assert(sizeof(struct thunk_gate_metaclass) ==
    sizeof(struct thunk_metaclass) + THUNK_GATE_NRELOCS * sizeof(thunk_reloc_t),
    "Invalid gate metaclass storage");

static_assert(offsetof(struct thunk_gate_metaclass, relocs) ==
    offsetof(struct thunk_metaclass, relocs),
    "Invalid gate metaclass relocs offset");

/**
 * Static descriptor for the thunk gate metaclass;
 */
static struct thunk_gate_metaclass thunk_gate_meta_storage = {
        .template = THUNK_TEMPLATE(gate),
        .template_end = THUNK_TEMPLATE_END(gate),
        .relocs_count = THUNK_GATE_NRELOCS,
        .relocs = {
                THUNK_REL_INITIALIZER(MOV_IMM, THUNK_PP(gate, token_base_0)),
                THUNK_REL_INITIALIZER(MOV_IMM, THUNK_PP(gate, token_base_16)),
                THUNK_REL_INITIALIZER(MOV_IMM, THUNK_PP(gate, token_base_32)),
                THUNK_REL_INITIALIZER(ADR, THUNK_PP(gate, data_offset)),
#ifdef THUNK_LARGE_TOKEN_SPACE
                THUNK_REL_INITIALIZER(MOV_IMM, THUNK_PP(gate, token_base_48)),
#endif
        },
};

struct thunk_gate_metaclass *thunk_gate_meta = &thunk_gate_meta_storage;

void
thunk_arch_gate_reloc_token_space(struct thunk_class *gate,
    thunk_token_t token_space)
{
        ptraddr_t tk_space_base = (ptraddr_t)token_space;

        gate->reloc_data[0].u16 = tk_space_base & 0xffff;
        gate->reloc_data[1].u16 = (tk_space_base >> 16) & 0xffff;
        gate->reloc_data[2].u16 = (tk_space_base >> 32) & 0xffff;
#ifdef THUNK_LARGE_TOKEN_SPACE
        gate->reloc_data[4].u16 = tk_space_base >> 48;
#else
        assert((tk_space_base >> 48) == 0 && "Invalid token space base");
#endif
}

void
thunk_arch_gate_reloc_data_offset(struct thunk_class *gate, size_t offset)
{
        gate->reloc_data[3].u32 = offset;
}
