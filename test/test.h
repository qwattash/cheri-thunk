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

#include <cheriintrin.h>
#include <stdio.h>

// BSD specific
#include <machine/cherireg.h>

#define assert_true(cond, msg) do {             \
        if (!(cond)) {                          \
                fprintf(stderr, "%s\n", (msg)); \
                abort();                        \
        }                                       \
} while (0)

#define assert_cap(cond, cap, msg) do {                                 \
        if (!(cond)) {                                                  \
                fprintf(stderr, "%s: %#p\n", (msg), (cap));             \
                abort();                                                \
        }                                                               \
} while (0)

#define assert_cap_pred(pred, cap, msg)         \
        assert_cap(pred((cap)), (cap), (msg))

#define assert_cap_valid(cap, msg)                \
        assert_cap_pred(cheri_is_valid, cap, msg)

#define assert_cap_len(cap, expect, msg) do {                           \
        if (cheri_length_get((cap)) != expect) {                        \
                fprintf(stderr, "%s: expected %#lx, %#p\n", msg, expect, cap); \
                abort();                                                \
        }                                                               \
} while (0)

#define assert_cap_exact_perms(cap, expect, msg) do {               \
        if (cheri_perms_get((cap)) != (expect)) {                   \
                fprintf(stderr, "%s: %#p perms:\n", (msg), (cap));  \
                test_dump_perms(cap);                               \
                abort();                                            \
        }                                                           \
} while (0)

#define assert_cap_perms_clear(cap, perms, msg) do {                    \
        if (cheri_perms_get((cap)) & (perms)) {                         \
                fprintf(stderr, "%s: %#p has permissions %s:\n",        \
                    (msg), (cap), (#perms));                            \
                test_dump_perms(cap);                                   \
                abort();                                                \
        }                                                               \
} while (0)

#define assert_cap_perms_set(cap, perms, msg) do {                      \
        if ((cheri_perms_get((cap)) & (perms)) != (perms)) {            \
                fprintf(stderr, "%s: %#p lacks permissions %s:\n",      \
                    (msg), (cap), (#perms));                            \
                test_dump_perms(cap);                                   \
                abort();                                                \
        }                                                               \
} while (0)

static inline void
test_dump_perms(void *cap)
{
        int perms = cheri_perms_get(cap);

        fprintf(stderr, "Found perms:\n");
#if defined(__aarch64__)
        if (perms & CHERI_PERM_GLOBAL)
                fprintf(stderr, "\tPERM_GLOBAL\n");
        if (perms & CHERI_PERM_EXECUTIVE)
                fprintf(stderr, "\tPERM_EXECUTIVE\n");
        if (perms & CHERI_PERM_SW0)
                fprintf(stderr, "\tPERM_SW0\n");
        if (perms & CHERI_PERM_SW1)
                fprintf(stderr, "\tPERM_SW1\n");
        if (perms & CHERI_PERM_SW2)
                fprintf(stderr, "\tPERM_SW2\n");
        if (perms & CHERI_PERM_SW3)
                fprintf(stderr, "\tPERM_SW3\n");
        if (perms & CHERI_PERM_MUTABLE_LOAD)
                fprintf(stderr, "\tPERM_MUTABLE_LOAD\n");
        if (perms & CHERI_PERM_COMPARTMENT_ID)
                fprintf(stderr, "\tPERM_COMPARTMENT_ID\n");
        if (perms & CHERI_PERM_BRANCH_SEALED_PAIR)
                fprintf(stderr, "\tPERM_BR_SEALED_PAIR\n");
        if (perms & CHERI_PERM_INVOKE)
                fprintf(stderr, "\tPERM_INVOKE\n");
        if (perms & CHERI_PERM_SYSTEM)
                fprintf(stderr, "\tPERM_SYSTEM\n");
        if (perms & CHERI_PERM_UNSEAL)
                fprintf(stderr, "\tPERM_UNSEAL\n");
        if (perms & CHERI_PERM_SEAL)
                fprintf(stderr, "\tPERM_SEAL\n");
        if (perms & CHERI_PERM_STORE_LOCAL_CAP)
                fprintf(stderr, "\tPERM_SL\n");
        if (perms & CHERI_PERM_STORE_CAP)
                fprintf(stderr, "\tPERM_SC\n");
        if (perms & CHERI_PERM_LOAD_CAP)
                fprintf(stderr, "\tPERM_LC\n");
        if (perms & CHERI_PERM_EXECUTE)
                fprintf(stderr, "\tPERM_X\n");
        if (perms & CHERI_PERM_STORE)
                fprintf(stderr, "\tPERM_STORE\n");
        if (perms & CHERI_PERM_LOAD)
                fprintf(stderr, "\tPERM_LOAD\n");
#else
#error "unsupported arch"
#endif
}
