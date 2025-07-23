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

#include <machine/cherireg.h>
#include "arch/thunk-asm.h"

#if defined(__riscv__)

#error TODO

#elif defined(__aarch64__)

#define HELLO_DATA_PERMS                            \
    (CHERI_PERM_EXECUTE | CHERI_PERM_STORE |        \
    CHERI_PERM_STORE_CAP | CHERI_PERM_LOAD_CAP)

/*
 * This demo thunk is not particularly interesting,
 * it just returns a read-only pointer to the data.
 */
THUNK(hello_thunk)
1:  // Patch #1 data start offset
    adr     c0, #0
    // Compute remaining size
    gclen   x11, c0
    gcoff   x10, c0
    sub     x11, x11, x10
    // Create RO data capability
    scbndse c0, c0, x11
    mov     x10, #(HELLO_DATA_PERMS)
    clrperm c0, c0, x10

    // INVARIANT: no capabilities leaked
    ret
ENDTHUNK(hello_thunk)

THUNK_DEF_PATCH_POINT(hello_thunk, data_offset, 1b)

#else
#error "Unsupported architecture"
#endif
