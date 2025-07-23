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

#define	PATCH_INS(buffer, offset)               \
        ((uint32_t *)((buffer) + (offset)))

#define	PATCH_OFF(tramp, name)	({                      \
        extern const int32_t patch_##tramp##_##name;    \
        size + patch_##tramp##_##name;                  \
})

#define PATCH_MOV(buf, offset, value)               \
        do {                                        \
                uint32_t _value = (value);          \
                _value = ((_value & 0xffff) << 5);  \
                *PATCH_INS(buf, offset) |= _value;  \
        } while (0)

#define	PATCH_ADR(buf, offset, target)                  \
        do {                                            \
                int32_t _offset = (offset);             \
                int32_t _value = (target) - _offset;    \
                _value =                                \
                        ((_value & 0x3) << 29) |        \
                        ((_value & 0x1ffffc) << 3);     \
                *PATCH_INS(buf, _offset) |= _value;     \
        } while (0)

