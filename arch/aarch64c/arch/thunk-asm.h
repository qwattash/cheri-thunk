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

#include <machine/asm.h>

#include "arch/thunk-patch.h"

/* lifted from rtld c18n trampoline generation */

#define	THUNK(tname)                                        \
        .section .rodata;                                   \
        .globl _THUNK_SYM(tname);                           \
        .type _THUNK_SYM(tname),#object; _THUNK_SYM(tname):

#define	ENDTHUNK(tname)                                  \
        .global _THUNK_END_SYM(tname);                   \
        .type _THUNK_END_SYM(tname),#object;             \
        .size _THUNK_END_SYM(tname), 1;                  \
        _THUNK_END_SYM(tname):                           \
        EEND(_THUNK_SYM(tname));

#define	THUNK_PP_LABEL(tname, label)                     \
        .globl _THUNK_PATCH(tname, label);               \
        .type _THUNK_PATCH(tname, label),#object;        \
        .size _THUNK_PATCH(tname, label), 4;             \
        _THUNK_PATCH(tname, label):
