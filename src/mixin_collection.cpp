// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include <dynamix/domain.hpp>
#include <dynamix/mixin_collection.hpp>
#include <dynamix/exception.hpp>
#include <algorithm>

using namespace std;

namespace dynamix
{

using namespace internal;

mixin_collection::mixin_collection()
{
}

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
}

mixin_collection::mixin_collection(const mixin_type_info_vector& mixins)
    : _mixins(build_available_mixins_from(mixins))
    , _compact_mixins(mixins)
{
}

void mixin_collection::check_valid_mixin(const mixin_type_info& mixin_info)
{
    // this shouldn't be possible without a bug in the library
    // hence no throw
    I_DYNAMIX_ASSERT(mixin_info.is_valid());
    I_DYNAMIX_MAYBE_UNUSED(mixin_info);
}

void mixin_collection::add(mixin_id id)
{
    // could be that the input parameter has been instantiated from a different module
    // that's why get the info that's actually in the domain, instead of just using an
    // argument of type mixin_type_info
    const mixin_type_info& info = domain::instance().mixin_info(id);

    if(_mixins[id])
    {
        // we already have this
        I_DYNAMIX_ASSERT(std::find(_compact_mixins.begin(), _compact_mixins.end(), &info) != _compact_mixins.end());
        return;
    }

    _mixins[id] = true;
    _compact_mixins.push_back(&info);
}

void mixin_collection::remove(mixin_id id)
{
    // could be that the input parameter has been instantiated from a different module
    // that's why get the info that's actually in the domain, instead of just using an
    // argument of type mixin_type_info
    const mixin_type_info& info = domain::instance().mixin_info(id);

    I_DYNAMIX_ASSERT(_mixins[id]); // should have it to remove it
    _mixins[id] = false;

    mixin_type_info_vector::iterator i = find(_compact_mixins.begin(), _compact_mixins.end(), &info);
    I_DYNAMIX_ASSERT(i != _compact_mixins.end());
    _compact_mixins.erase(i);
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
    rebuild_from_compact_mixins();
}

} // namespace dynamix
