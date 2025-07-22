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

#include <machine/asm.h>

#include "arch/thunk-patch.h"

/* lifted from rtld c18n trampoline generation */
#define	__CONCAT(a, b) a ## b
#define	_CONCAT(a, b) __CONCAT(a, b)

#define	_THUNK_END_SYM(tname) _CONCAT(end_, _THUNK_SYM(tname))

#define	THUNK(tname)                                        \
        .section .rodata;                                   \
        .globl _THUNK_SYM(tname);                           \
        .type _THUNK_SYM(tname),#object; _THUNK_SYM(tname):

#define	ENDTHUNK(tname)                                                 \
        _THUNK_END_SYM(tname):                                          \
        EEND(_THUNK_SYM(tname));                                        \
        .section .rodata;                                               \
        .globl _THUNK_SZ_SYM(tname);                                    \
        .align 3;                                                       \
        .type _THUNK_SZ_SYM(tname),#object;                             \
        .size _THUNK_SZ_SYM(tname), 8;                                  \
        _THUNK_SZ_SYM(tname):                                           \
        .quad _THUNK_END_SYM(tname) - _THUNK_SYM(tname)

#define	THUNK_DEF_PATCH_POINT(tname, name, label)                      \
        .section .rodata; .globl _THUNK_PATCH(tname, name); .align 2;  \
        .type _THUNK_PATCH(tname, name),#object;                       \
        .size _THUNK_PATCH(tname, name), 4;                            \
        _THUNK_PATCH(tname, name): .word label - _THUNK_SYM(tname)
