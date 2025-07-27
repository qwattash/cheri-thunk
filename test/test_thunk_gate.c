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

        thunk_gate_class_t test_gate_type = thunk_gateclass_create(
            sizeof(struct test_data));
        // TODO Verify gate class invariants

        thunk_token_t root_token = thunk_gateclass_token(test_gate_type);
        assert_cap_valid(root_token, "Invalid gate root token");
        assert_cap_pred(cheri_is_unsealed, root_token,
            "Sealed gate root token");
        assert_cap_len(root_token, sizeof(struct test_data),
            "Invalid root token length");

        thunk_gate_t test_gate = thunk_gate_alloc(test_gate_type);

#ifdef THUNK_AUTH_MODE_PERMS
        assert_cap_perms_set(thunk_object_unwrap(test_gate.obj),
            CHERI_PERM_SW_THUNK,
            "Gate object lacks SW_THUNK permission");
#endif
        assert_true(thunk_gate_auth(test_gate),
            "Thunk object authentication failed");

        // Now get the full data pointer using the root token
        struct test_data *p = thunk_gate_invoke(test_gate, root_token);
        assert_cap_valid(p, "Invalid full object pointer");
        assert_cap_pred(cheri_is_unsealed, p, "Sealed full object pointer");
        assert_cap_len(p, sizeof(struct test_data),
            "Invalid full object length");
        assert_cap_exact_perms(p, DEFAULT_PERMS_MASK,
            "Invalid full object perms");

        // Get a field token
        thunk_token_t test_public_token = thunk_token_for(
            struct test_data, public_value, root_token);
        long *value = thunk_gate_invoke(test_gate, test_public_token);
        assert_cap_valid(value, "Invalid public_value pointer");
        assert_cap_pred(cheri_is_unsealed, value,
            "Sealed public_value pointer");
        assert_cap_len(value, sizeof(long), "Invalid public_value ptr length");
        assert_cap_exact_perms(value, DEFAULT_PERMS_MASK,
            "Invalid public_value ptr perms");

        thunk_gate_free(test_gate_type, test_gate);
        thunk_gateclass_destroy(test_gate_type);

        return (0);
}
