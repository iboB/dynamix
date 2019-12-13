// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include <dynamix/object_type_mutation.hpp>

using namespace std;

namespace dynamix
{

using namespace internal;

object_type_mutation::object_type_mutation()
    : _source(nullptr)
{
}

object_type_mutation::object_type_mutation(const mixin_collection* src)
    : _source(src)
{
}

void object_type_mutation::normalize()
{
    // intentionally not sorting both containers and not using set_intersection
    // they should be small so this o(a*b) algorithm should be fine

    mixin_type_info_vector& to_add = _adding._compact_mixins;
    mixin_type_info_vector& to_remove = _removing._compact_mixins;

    for(mixin_type_info_vector::iterator add = to_add.begin(); add!=to_add.end(); ++add)
    {
        for(mixin_type_info_vector::iterator rem = to_remove.begin(); rem!=to_remove.end(); ++rem)
        {
            if(*add == *rem)
            {
                add = to_add.erase(add);
                to_remove.erase(rem);

                // the for ++ will increase add, so decrement it here
                if(add == to_add.end())
                {
                    goto break2;
                }

                break;
            }
        }
    }

break2:

    _adding.rebuild_from_compact_mixins();
    _removing.rebuild_from_compact_mixins();
}

void object_type_mutation::clear()
{
    _adding.clear();
    _removing.clear();
}

} // namespace dynamix
