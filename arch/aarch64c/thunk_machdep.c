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
#include <stdlib.h>
#include <string.h>

#include "thunk.h"

static inline thunk_jit_t
patch_point(thunk_jit_t code_buf, size_t offset)
{
        uintptr_t target = (uintptr_t)code_buf;

        target += offset;
        assert(target % sizeof(uint32_t) == 0 && "Misaligned patch");

        return ((thunk_jit_t)target);
}

int
thunk_compile(thunk_jit_t code_buf, const struct thunk_class *tc)
{
        const struct thunk_metaclass *mc = tc->mc;
        int index = 0;

        memset(code_buf, 0, cheri_representable_length(mc->code_size));
        memcpy(code_buf, mc->template, mc->code_size);

        while (index < mc->relocs_count) {
                thunk_jit_t pp = patch_point(code_buf,
                    mc->relocs[index].offset);

                switch (mc->relocs[index].type) {
                case THUNK_REL_MOV_IMM: {
                        uint32_t value = tc->reloc_data[index].u32;
                        value = (value & 0xffff) << 5;
                        *pp |= value;
                        break;
                }
                case THUNK_REL_ADR: {
                        // XXX assert representable distance
                        int32_t value = (ptraddr_t)code_buf + (
                            tc->reloc_data[index].i32 - (ptraddr_t)pp);
                        value = ((value & 0x3) << 29) |
                            ((value & 0x1ffffc) << 3);
                        *pp |= value;
                        break;
                }
                default:
                        assert(0 && "Unsupported thunk relocation");
                        memset(code_buf, 0, mc->code_size);
                        return (1);
                }
                index++;
        }

        return (0);
}
