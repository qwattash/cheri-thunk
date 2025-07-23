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
#include <stdlib.h>

#include "hello.h"
#include "arch/thunk-patch.h"

#if !defined(__riscv__) && !defined(__aarch64__)
#error "Unsupported architecture"
#endif

#define HELLO_NRELOCS 1

THUNK_DECL_TEMPLATE(hello_thunk);
THUNK_DECL_PATCH_POINT(hello_thunk, data_offset);

/**
 * The Hello Thunk is a demo thunk class that embeds
 * a fixed-size string buffer.
 *
 * The thunk simply returns a read-only view of the buffer.
 * after initialisation.
 */
struct thunk_metaclass *hello_meta;
struct thunk_class *hello_class;

struct hello_data {
        char message[256];
};

/*
 * XXX this could alternatively be done with a linker set?
 * there is a bunch of code here that could be generalised..
 */
__attribute__((constructor))
static void
hello_init()
{
        const size_t data_align = cheri_representable_alignment_mask(
            sizeof(struct hello_data));
        size_t data_offset;

        hello_meta = thunk_level_malloc(sizeof(*hello_meta) +
            HELLO_NRELOCS * sizeof(thunk_reloc_t), THUNK_LEVEL_PRIVATE);
        hello_meta->template = THUNK_TEMPLATE(hello_thunk);
        hello_meta->code_size = THUNK_TEMPLATE_SIZE(hello_thunk);
        hello_meta->relocs_count = HELLO_NRELOCS;
        // Init thunk relocation descriptors
#if defined(__aarch64__)
        hello_meta->relocs[0].type = THUNK_REL_MOV_IMM;
        hello_meta->relocs[0].offset = THUNK_PATCH_POINT(hello_thunk,
            data_offset);
#endif

        data_offset = cheri_align_up(hello_meta->code_size, data_align);

        hello_class = thunk_level_malloc(sizeof(*hello_class) +
            HELLO_NRELOCS * sizeof(thunk_reloc_data_t), THUNK_LEVEL_PRIVATE);
        hello_class->mc = hello_meta;
        hello_class->object_size = cheri_representable_length(
            data_offset + sizeof(struct hello_data));
        hello_class->ctor = NULL;
        hello_class->dtor = NULL;
        // Bind relocations to the actual values for this class.
#if defined(__aarch64__)
        hello_class->reloc_data[0].u32 = data_offset;
#endif

}

hello_object_t
hello_create()
{
        thunk_object_t hello_obj = thunk_malloc(hello_class);

        return ((hello_object_t)hello_obj);
}

void
hello_destroy(hello_object_t obj)
{
        thunk_free(hello_class, obj._o);
}
