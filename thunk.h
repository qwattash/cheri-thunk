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

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>

#include <sys/cdefs.h>

#include "arch/thunk.h"

#ifndef static_assert
// static_assert is introduced in C23, _Static_assert in C11
#define static_assert _Static_assert
#endif

typedef const uint8_t* thunk_token_t;

/**
 * Private type representing a thunk object.
 *
 * This maintains the invariant that the __inner pointer is always sealed.
 * This prevents uncontrolled casts to different pointer types.
 */
typedef struct {
        // XXX in debug mode, remember thunk data size
        void *__inner;
} thunk_object_t;

static inline thunk_object_t
_thunk_object_wrap(void *ptr)
{
        thunk_object_t obj = { .__inner = ptr };

        assert(cheri_is_valid(ptr) && "thunk_object_wrap: invalid pointer");
        assert(cheri_is_sealed(ptr) && "thunk_object_wrap: unsealed pointer");
        // assert(cheri_get_mode);

        return (obj);
}

#define thunk_object_unwrap(obj) (((thunk_object_t *)&(obj))->__inner)
#define thunk_object_wrap(ptr) _thunk_object_wrap((void *)ptr)
#define THUNK_NULLOBJ (thunk_object_t){ .__inner = NULL }

/**
 * Defines a shareability level for allocated memory.
 */
enum thunk_level {
        /* Compartment-private */
        THUNK_LEVEL_PRIVATE,
        /* Shareable */
        THUNK_LEVEL_SHAREABLE,
};

typedef enum thunk_level thunk_level_t;

#define THUNK_METACLASS_HEADER                                \
        /* Template code, not runnable */                     \
        thunk_template_t template;                            \
        /* Template code end, not runnable XXX DEBUG ONLY */  \
        thunk_template_t template_end;                        \
        /* Number of thunk relocations */                     \
        unsigned int relocs_count

/**
 * A thunk metaclass describes the template block associated with a
 * specific thunk class.
 *
 * Thunk metaclasses may be used to implement different semantics associated
 * to data.
 */
struct thunk_metaclass {
        THUNK_METACLASS_HEADER;
        /* Describe machine-dependent patch points within the template */
        thunk_reloc_t relocs[];
};

static inline size_t
thunk_code_size(const struct thunk_metaclass *mc)
{
        size_t size = (uintptr_t)mc->template_end - (uintptr_t)mc->template;
        assert(cheri_length_get(mc->template) == size &&
            "Unexpected code size");

        return (size);
}

/**
 * A thunk class binds a specific metaclass to a type of data.
 *
 * The template operates on the data specified by the type by
 * binding to the metaclass patch points.
 */
struct thunk_class {
        /* Metaclass describing the template */
        struct thunk_metaclass *mc;
        /* Total size */
        size_t object_size;
        /* Constructor (runs in the thunk compartment) */
        void (*ctor)(void *);
        /* Destructor (runs in the thunk compartment) */
        void (*dtor)(void *);
        /* Thunk token space for this class */
        void *token_space;
        /* Resolved values for the thunk patch descriptors, matching order */
        thunk_reloc_data_t reloc_data[];
};

/**
 * Wrapper around the system malloc interface.
 *
 * This allocates memory subject to capability flow enforcement.
 */
void *thunk_level_malloc(size_t size, thunk_level_t level);
void thunk_level_free(void *);

/**
 * Create an instance of the given thunk class.
 *
 * This creates a concrete thunk object with data associated and initialised.
 */
thunk_object_t thunk_malloc(struct thunk_class *tc);

/**
 * Destroy an instance of a thunk class.
 *
 * The thunk_object must be valid and sealed.
 */
void thunk_free(struct thunk_class *tc, thunk_object_t t_obj);

/**
 * Compile a thunk class into the code buffer of a thunk object.
 *
 * Note that the thunk allocation is always RWX.
 */
int thunk_compile(thunk_jit_t code_buf, const struct thunk_class *tc);

/**
 * Executable memory allocation hooks.
 * These must be defined at link-time, the default weak symbols will abort();
 */
void *thunk_xmalloc(size_t size);
void thunk_xfree(void *ptr);

/**
 * Opaque type for thunk gate class sealed capabilities.
 *
 * These capabilities authorise the creation of gate thunks
 * of a specific type.
 */
typedef struct {
        struct thunk_class *_class;
        thunk_token_t root_token;
} thunk_gate_t;

#define THUNK_NULLGATE ((thunk_gate_t){ ._class = NULL, .root_token = NULL })

/**
 * Function signature of the thunk gate object.
 *
 * This can be used to invoke a gate thunk object with a corresponding token.
 */
typedef void *(*thunk_gate_fn_t)(thunk_token_t);

/**
 * Set the token space relocations for a given thunk gate class.
 * XXX could inline
 */
void thunk_arch_gate_reloc_token_space(struct thunk_class *gate,
                                       thunk_token_t token_space);

/**
 * Set the data offset relocations for a given thunk gate class.
 * XXX could inline
 */
void thunk_arch_gate_reloc_data_offset(struct thunk_class *gate,
                                       ptraddr_t offset);

/**
 * Create a new gate thunk class with a given object size.
 *
 * Gate thunk classes are associated to a data type.
 * For each type, they provide a thunk token space that authorises
 * the bearer to access a sub-object range of the data type,
 * according to the permission bits authorised by the token.
 * The code in gate thunks must be trusted and the sealed gate thunk
 * capabilities are marked with the CHERI_PERM_SW_THGATE permission bit.
 *
 * XXX can I just return the root gate token instead and keep the class
 * to myself?
 *
 * The gate thunk mediates access to an underlying data structure.
 * The access is authorised using a special set of capabilities in
 * a thunk token space.
 * Each gate class is associated with an unique thunk token space,
 * which authorises access to the gate object data.
 * The thunk tokens are normal unsealed capabilities, however they
 * are mapped to guard pages, therefore they are useless for memory
 * access. Instead we leverage the ability to monotonically modify
 * thunk tokens to allow the delegation of access to thunk gates.
 */
thunk_gate_t thunk_gate_create(size_t size);

/**
 * Destroy a thunk gate class.
 *
 * This requires special care. If the gate class is destroyed,
 * any thunk allocated via the gate will be impossible to free.
 * XXX-AM: In debug mode, keep a reference count to check
 */
void thunk_gate_destroy(thunk_gate_t gate);

/**
 * Allocate a thunk object for a given gate.
 */
thunk_object_t thunk_gate_alloc_object(thunk_gate_t gate);

/**
 * Free a thunk object.
 *
 * Note that we need a good way to authorise this.
 */
void thunk_gate_free_object(thunk_gate_t gate, thunk_object_t obj);

/**
 * Invoke the gate object with the given token
 */
static inline void *
thunk_gate_ref(thunk_object_t gobj, thunk_token_t tok)
{
        return (((thunk_gate_fn_t)thunk_object_unwrap(gobj))(tok));
}
