// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/alloc_util.hpp>
#include <dynamix/mixin_info.hpp>
#include <doctest/doctest.h>

static_assert(dynamix::util::next_multiple(7, 3) == 9);
static_assert(dynamix::util::next_multiple(8, 4) == 8);

TEST_CASE("external mixin data") {
    const dynamix::byte_size_t alignments[] = {0, 1, 2, 4, 8, 16, 32, 64, 128};
    for (auto a : alignments) {
        for (dynamix::byte_size_t is = 1; is < 2; ++is) {
            const auto s = is * a;

            dynamix::mixin_info bd;
            bd.set_size_alignment(s, a);

            // proper alignment
            CHECK(bd.obj_buf_alignment_and_mixin_offset >= a);

            // room for obj pointer
            CHECK(bd.obj_buf_alignment_and_mixin_offset >= dynamix::sizeof_ptr);

            // room for mixin
            CHECK(bd.obj_buf_size - bd.obj_buf_alignment_and_mixin_offset >= s);
        }
    }
}
