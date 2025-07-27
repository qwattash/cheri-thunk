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

#include "thunk-gate.h"

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
 * Validate and unwrap the gate entrypoint.
 *
 * If validation fails, NULL is returned.
 */
static inline
thunk_gate_fn_t
thunk_gate_unwrap(thunk_gate_t gate)
{
#ifdef THUNK_AUTH_MODE_PERMS
        void *entry = thunk_object_unwrap(gate.obj);

        if (cheri_is_sealed(entry) &&
            (cheri_perms_get(entry) & CHERI_PERM_SW_THUNK)) {
                return (entry);
        }

        return (NULL);
#else
#error "Unsupported thunk authentication mode"
#endif
}

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


thunk_gate_class_t
thunk_gateclass_create(size_t size)
{
        const size_t data_align = ~cheri_representable_alignment_mask(size) + 1;
        size_t data_offset;
        struct thunk_gate_class *gate_class;
        struct thunk_class *tclass;

        data_offset = cheri_align_up(thunk_code_size(thunk_gate_meta),
            data_align);

        // XXX really local?
        gate_class = thunk_level_malloc(sizeof(*gate_class) +
            thunk_gate_meta->relocs_count * sizeof(thunk_reloc_data_t),
            THUNK_LEVEL_PRIVATE);
        if (gate_class == NULL)
                return (THUNK_NULL_GATECLASS);

        gate_class->requested_size = size;
        gate_class->token_space = token_space_alloc(size);
        if (gate_class->token_space == NULL) {
                thunk_level_free(gate_class);
                return (THUNK_NULL_GATECLASS);
        }

        tclass = &gate_class->thunk_class;
        tclass->mc = thunk_gate_meta;
        tclass->object_size = cheri_representable_length(data_offset + size);
        tclass->ctor = NULL;
        tclass->dtor = NULL;

        thunk_arch_gate_reloc_data_offset(tclass, data_offset);
        thunk_arch_gate_reloc_token_space(tclass, gate_class->token_space);

        pthread_mutex_lock(&gate_head_mutex);
        TAILQ_INSERT_HEAD(&gate_head, gate_class, gate_list);
        pthread_mutex_unlock(&gate_head_mutex);

        // XXX we can wrap the gate class into another gate thunk
        // which can be unsealed using a special token we keep for ourselves.
        return ((thunk_gate_class_t){ .class = gate_class });
}

void
thunk_gateclass_destroy(thunk_gate_class_t gc)
{

}

thunk_token_t
thunk_gateclass_token(thunk_gate_class_t gc)
{
        // XXX auth gateclass
        const struct thunk_gate_class *gate_class = gc.class;
        thunk_token_t root_token;

        root_token = cheri_perms_and(gate_class->token_space,
            THUNK_TOKEN_MAX_PERMS);
        root_token = cheri_bounds_set_exact(root_token,
            gate_class->requested_size);

        return (root_token);
}

/**
 * XXX-AM: Note that this is currently boring but we will
 * incrementally do more things.
 * In particular, we need to authorise the malloc and the
 * class pointer needs to be sealed.
 *
 * We may want to deal with initialisation as well.
 */
thunk_gate_t
thunk_gate_alloc(thunk_gate_class_t gc)
{
        // XXX auth gateclass
        struct thunk_gate_class *gate_class = gc.class;
        thunk_gate_t gate;

        gate.obj = thunk_malloc(&gate_class->thunk_class);
        return (gate);
}

void
thunk_gate_free(thunk_gate_class_t gc, thunk_gate_t gate)
{

}

void *
thunk_gate_invoke(thunk_gate_t gate, thunk_token_t tok)
{
        thunk_gate_fn_t gate_entry = thunk_gate_unwrap(gate);

        assert(gate_entry != NULL && "Invalid thunk gate");
        return (gate_entry(tok));
}

bool
thunk_gate_auth(thunk_gate_t gate)
{
        return (thunk_gate_unwrap(gate) != NULL);
}
