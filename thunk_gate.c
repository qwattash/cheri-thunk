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
#include <pthread.h>

#include <sys/mman.h>
#include <sys/queue.h>
#include <cheri/cherireg.h>

#include "thunk.h"

/**
 * Private data associated to gate classes.
 *
 * We keep a private list of allocated gate classes that we can lookup.
 */
struct thunk_gate_class {
        /* Link in the gate_head list */
        TAILQ_ENTRY(thunk_gate_class) gate_list;
        /*
         * Root capability for the token space.
         * Note that this has the SW_PERM_VMEM and may span a larger
         * chunk of memory than the requrested object size.
         */
        thunk_token_t token_space;
        /* Requested object size */
        size_t requested_size;
        /* Thunk class associated to a specific gate type */
        struct thunk_class thunk_class;
};

/* Global thunk gate metaclass */
extern struct thunk_metaclass *thunk_gate_meta;

/* Global gate thunk class list */
static pthread_mutex_t gate_head_mutex = PTHREAD_MUTEX_INITIALIZER;
static TAILQ_HEAD(thunk_gate_head, thunk_gate_class) gate_head =
    TAILQ_HEAD_INITIALIZER(gate_head);

/**
 * Allocate a new chunk of token space.
 *
 * The function returns the root token for the token space.
 *
 * This is currently stupid and will just bump up the requested size to
 * the nearest page boundary, wasting address space.
 * XXX We could try to be smart about it...
 */
static thunk_token_t
token_space_alloc(size_t size)
{
        thunk_token_t space;

        space = mmap(NULL, size, PROT_NONE |
            PROT_MAX(PROT_READ | PROT_WRITE | PROT_EXEC | PROT_CAP),
            MAP_GUARD, -1, 0);
        if (space == MAP_FAILED)
                return (NULL);

        return (space);
}

static void
token_space_free(thunk_token_t token)
{
        // XXX-AM: TODO
}


thunk_gate_t
thunk_gate_create(size_t size)
{
        const size_t data_align = ~cheri_representable_alignment_mask(size) + 1;
        size_t data_offset;
        struct thunk_gate_class *gate_class;
        struct thunk_class *tc;
        thunk_token_t root_token;
        thunk_gate_t gate = THUNK_NULLGATE;

        data_offset = cheri_align_up(thunk_code_size(thunk_gate_meta),
            data_align);

        gate_class = thunk_level_malloc(sizeof(*gate_class) +
            thunk_gate_meta->relocs_count * sizeof(thunk_reloc_data_t),
            THUNK_LEVEL_PRIVATE);
        if (gate_class == NULL)
                return (gate);

        gate_class->requested_size = size;
        gate_class->token_space = token_space_alloc(size);
        if (gate_class->token_space == NULL) {
                thunk_level_free(gate_class);
                return (gate);
        }
        // We strip both SW_VMEM and EXECUTE.
        // This assumes that there is no code within the thunk data.
        root_token = cheri_perms_and(gate_class->token_space,
                                     THUNK_TOKEN_MAX_PERMS);
        root_token = cheri_bounds_set_exact(root_token, size);

        tc = &gate_class->thunk_class;
        tc->mc = thunk_gate_meta;
        tc->object_size = cheri_representable_length(data_offset + size);
        tc->ctor = NULL;
        tc->dtor = NULL;

        thunk_arch_gate_reloc_data_offset(tc, data_offset);
        thunk_arch_gate_reloc_token_space(tc, root_token);

        pthread_mutex_lock(&gate_head_mutex);
        TAILQ_INSERT_HEAD(&gate_head, gate_class, gate_list);
        pthread_mutex_unlock(&gate_head_mutex);

        // XXX we can wrap the gate class into another gate thunk
        // which can be unsealed using a special token we keep for ourselves.
        gate._class = &__builtin_no_change_bounds(gate_class->thunk_class);
        gate.root_token = root_token;

        return (gate);
}

void
thunk_gate_destroy(thunk_gate_t gate)
{

}

/**
 * XXX-AM: Note that this is currently boring but we will
 * incrementally do more things.
 * In particular, we need to authorise the malloc and the
 * class pointer needs to be sealed.
 *
 * We may want to deal with initialisation as well.
 */
thunk_object_t
thunk_gate_alloc_object(thunk_gate_t gate)
{
        return (thunk_malloc(gate._class));
}

void
thunk_gate_free_object(thunk_gate_t gate, thunk_object_t obj)
{

}
