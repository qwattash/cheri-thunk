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

/* lifted from rtld c18n trampoline generation */

/* Abstract internal symbol names */
#define _THUNK_PATCH(tname, name) patch_##tname##_##name
#define _THUNK_SYM(tname) thunk_##tname
#define _THUNK_SZ_SYM(tname) size_thunk_##tname

#define	THUNK_DECL_TEMPLATE(tname)                          \
        extern const uint32_t _THUNK_SYM(tname)[];          \
        extern const size_t _THUNK_SZ_SYM(tname)

#define	THUNK_DECL_PATCH_POINT(tname, name)             \
        extern const int32_t _THUNK_PATCH(tname, name)

/* Public visible symbol names */
#define THUNK_TEMPLATE(tname) _THUNK_SYM(tname)
#define THUNK_TEMPLATE_SIZE(tname) _THUNK_SZ_SYM(tname)
#define	THUNK_PATCH_POINT(tname, name) _THUNK_PATCH(tname, name)
