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

/* lifted from rtld c18n trampoline generation */

#ifndef __CONCAT
#define	_CONCAT1(a, b) a ## b
#define	__CONCAT(a, b) _CONCAT1(a, b)
#endif

/* Abstract internal symbol names */
#define	_THUNK_PATCH(tname, label) thunk_pp_label_##tname##_##label
#define	_THUNK_SYM(tname) thunk_##tname
#define	_THUNK_END_SYM(tname) __CONCAT(end_, _THUNK_SYM(tname))

#define	THUNK_DECL_TEMPLATE(tname)                          \
        extern const uint32_t _THUNK_SYM(tname)[];          \
        extern const uint32_t _THUNK_END_SYM(tname)[]

#define	THUNK_DECL_PATCH_POINT(tname, label)                \
        extern const uint32_t _THUNK_PATCH(tname, label)[]

/* Public visible symbol names */
#define	THUNK_TEMPLATE(tname) _THUNK_SYM(tname)
#define	THUNK_TEMPLATE_END(tname) _THUNK_END_SYM(tname)
#define	THUNK_PP(tname, label) ((ptraddr_t)_THUNK_PATCH(tname, label))
