// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include <dynamix/global.hpp>
#include <dynamix/mixin_type_info.hpp>

namespace dynamix
{
namespace internal
{
    available_mixins_bitset build_available_mixins_from(const mixin_type_info_vector& mixins)
    {
        available_mixins_bitset result;

        for (const mixin_type_info* mixin_info : mixins)
        {
            result[mixin_info->id] = true;
        }

        return result;
    }
}
}
