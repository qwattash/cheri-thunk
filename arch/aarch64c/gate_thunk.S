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

#define HELLO_DATA_PERMS                            \
    (CHERI_PERM_EXECUTE | CHERI_PERM_STORE |        \
    CHERI_PERM_STORE_CAP | CHERI_PERM_LOAD_CAP)

/**
  * The thunk gate.
  *
  * This code is designed to be small and branch-less.
  * Furthermore, the following invariants must be satisfied:
  *  - No stack usage. If stack is used, it must be cleared.
  *  - No capabilities from the thunk may leak after return.
  *
  * XXX-AM: Maybe we can optimise this to remove some instructions.
  * - The token space may be allocated in the bottom 4G of AS,
  *   so that we can use a 32bit relocation.
  * - The token space may be aligned, at least as much as the
  *   data type. Given that we must know the data type size
  *   when we generate this code, we may omit some bits
  *   from the relocation. If we further align the token space,
  *   we may be able to cut even more.
  *
  * The gate expects a token as its argument (c0) and returns a pointer.
  * void *thunk_gate(thunk_token_t token);
  */
THUNK(gate)
    // Check tag on token
    chktgd  c0

    // Patch 1-4: token space base address
    // Note: While the token base address could be a full 64bit
    // value, we assume that thunk tokens are always allocated
    // in the user memory range in an 48bit virtual address space.
    // As a result, the top 16 bits will always be zero.
    // This saves us an instruction and a patch point for
    // token_base_48.
THUNK_PP_LABEL(gate, token_base_0)
    mov     x10, #0
THUNK_PP_LABEL(gate, token_base_16)
    movk    x10, #0, lsl #16
THUNK_PP_LABEL(gate, token_base_32)
    movk    x10, #0, lsl #32
#ifdef THUNK_LARGE_TOKEN_SPACE
THUNK_PP_LABEL(gate, token_base_48)
    movk    x10, #0, lsl #48
#endif

    // Check that the token is in the right token space
    gcbase  x11, c0
    // ccmp    x11, x10, #0, cs
    sub     x11, x11, x10   // member token offset
    gclen   x12, c0
    gcperm  x13, c0

    // Patch 5: data start offset
THUNK_PP_LABEL(gate, data_offset)
    adr     c0, #0

    csel    c0, c0, czr, cs
    add     c0, c0, x11
    scbndse c0, c0, x12
    mvn     x13, x13
    clrperm c0, c0, x13

    ret
ENDTHUNK(gate)
