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
#include <string.h>

#include <machine/cherireg.h>

#include "thunk-gate.h"
#include "test.h"

struct test_data {
        int private_value;
        long public_value;
};

#ifdef __aarch64__
#define DEFAULT_PERMS_MASK                                          \
        (CHERI_PERM_LOAD | CHERI_PERM_STORE | CHERI_PERM_LOAD_CAP | \
         CHERI_PERM_STORE_CAP | CHERI_PERM_GLOBAL |                 \
         CHERI_PERM_STORE_LOCAL_CAP | CHERI_PERM_MUTABLE_LOAD)
#endif

#ifdef THUNK_AUTH_MODE_PERMS
static void
check_system_malloc()
{
        /**
         * Normal malloc should have been hooked in the test run
         */
        void *test = malloc(128);

        assert_cap_valid(test, "Invalid system malloc");
        assert_cap_perms_clear(test, CHERI_PERM_SW_THUNK,
                               "System malloc returns SW_THUNK permission");

        free(test);
}
#endif

/**
 * Test the basic operation of the thunk gate library.
 */
int
main(int argc, char *argv[])
{

#ifdef THUNK_AUTH_MODE_PERMS
        check_system_malloc();
#endif

        thunk_gate_t test_gate = thunk_gate_create(sizeof(struct test_data));

        assert_cap_valid(test_gate.root_token, "Invalid gate root token");
        assert_cap_pred(cheri_is_unsealed, test_gate.root_token,
            "Sealed gate root token");
        assert_cap_len(test_gate.root_token, sizeof(struct test_data),
            "Invalid root token length");

        thunk_object_t data = thunk_gate_alloc_object(test_gate);

#ifdef THUNK_AUTH_MODE_PERMS
        assert_cap_perms_set(thunk_object_unwrap(data), CHERI_PERM_SW_THUNK,
            "Gate object lacks SW_THUNK permission");
#endif
        assert_true(thunk_gate_auth(data), "Thunk object authentication failed");

        // Now get the full data pointer using the root token
        struct test_data *p = thunk_gate_ref(data, test_gate.root_token);

        assert_cap_valid(p, "Invalid full object pointer");
        assert_cap_pred(cheri_is_unsealed, p, "Sealed full object pointer");
        assert_cap_len(p, sizeof(struct test_data),
            "Invalid full object length");
        assert_cap_exact_perms(p, DEFAULT_PERMS_MASK,
            "Invalid full object perms");

        thunk_gate_destroy(test_gate);

        return (0);
}
