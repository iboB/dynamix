// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include <dynamix/mixin_collection.hpp>

#include <dynamix/domain.hpp>

#include <algorithm>

namespace dynamix
{

using namespace internal;

mixin_collection::mixin_collection() noexcept = default;
mixin_collection::~mixin_collection() = default;

namespace
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
} // namespace

mixin_collection::mixin_collection(const mixin_type_info_vector& mixins)
    : _mixins(build_available_mixins_from(mixins))
    , _compact_mixins(mixins)
{
}

void mixin_collection::add(const mixin_type_info& info)
{
    if (has(info)) return; // we already have this
    _mixins[info.id] = true;
    _compact_mixins.push_back(&info);
}

void mixin_collection::add(mixin_id id)
{
    const mixin_type_info& info = domain::instance().mixin_info(id);
    add(info);
}

bool mixin_collection::remove(const mixin_type_info& info)
{
    if (!has(info)) return false;
    auto i = std::find(_compact_mixins.begin(), _compact_mixins.end(), &info);
    I_DYNAMIX_ASSERT(i != _compact_mixins.end());
    _compact_mixins.erase(i);
    return true;
}

bool mixin_collection::remove(mixin_id id)
{
    const mixin_type_info& info = domain::instance().mixin_info(id);
    return remove(info);
}

void mixin_collection::rebuild_from_compact_mixins()
{
    _mixins.reset();
    for (const mixin_type_info* info : _compact_mixins)
    {
        _mixins[info->id] = true;
    }
}

void mixin_collection::clear()
{
    _compact_mixins.clear();
    _mixins.reset();
}

} // namespace dynamix
