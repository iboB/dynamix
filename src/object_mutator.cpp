// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include <dynamix/object_mutator.hpp>
#include <dynamix/object_type_info.hpp>
#include <dynamix/mixin_type_info.hpp>
#include <dynamix/exception.hpp>
#include <dynamix/domain.hpp>
#include <dynamix/object.hpp>

namespace dynamix
{
namespace internal
{

object_mutator::object_mutator()
    : _target_type_info(nullptr)
    , _is_created(false)
{
}

object_mutator::object_mutator(const mixin_collection* source_mixins)
    : _mutation(source_mixins)
    , _target_type_info(nullptr)
    , _is_created(false)
{
}

void object_mutator::cancel()
{
    _mutation.clear();
    _target_type_info = nullptr;
    _is_created = false;
}

void object_mutator::create()
{
    DYNAMIX_ASSERT(_mutation._source);

    if(_is_created)
    {
        return;
    }
    _is_created = true;

    _mutation.normalize();

    domain& dom = domain::instance();
    dom.apply_mutation_rules(_mutation);

    // in case the rules broke it somehow
    _mutation.normalize();

    if(_mutation.empty())
    {
        // nothing to do
        return;
    }

    mixin_type_info_vector new_type_mixins;
    const mixin_type_info_vector& old_mixins = _mutation._source->_compact_mixins;
    new_type_mixins.reserve(_mutation._adding._compact_mixins.size() + old_mixins.size());

    new_type_mixins = _mutation._adding._compact_mixins;

    for (const mixin_type_info* mixin_info : old_mixins)
    {
        // intentionally using linear search instead of binary
        // cache locality makes it faster for small arrays
        if(!has_elem(_mutation._removing._compact_mixins, mixin_info))
        {
            // elements are part of the new type only if they're not removed
            new_type_mixins.push_back(mixin_info);
        }
    }

    if(new_type_mixins.empty())
    {
        _target_type_info = &object_type_info::null();
        return;
    }

    sort(new_type_mixins.begin(), new_type_mixins.end());

    // erase duplicates
    // there could be duplicates if we're adding something that's already there
    new_type_mixins.erase(std::unique(new_type_mixins.begin(), new_type_mixins.end()), new_type_mixins.end());

    _target_type_info = dom.get_object_type_info(new_type_mixins);

    if(_target_type_info->as_mixin_collection() == _mutation._source)
    {
        // since we allow adding of existing mixins it could be that this new type is
        // actually the mutatee's type
        _target_type_info = nullptr;
        return;
    }
}

void object_mutator::apply_to(object& obj) const
{
    DYNAMIX_THROW_UNLESS(_is_created, bad_mutation);
    DYNAMIX_ASSERT(_mutation._source);

    // we need to mutate only objects of the same type
    DYNAMIX_THROW_UNLESS(obj._type_info->as_mixin_collection() == _mutation._source, bad_mutation_source);

    if(!_target_type_info)
    {
        // this is an empty mutation
        return;
    }

    // shouldn't be trying to set the same type info
    // unless they're both null, which is covereted by the previous if
    DYNAMIX_ASSERT(obj._type_info != _target_type_info);

    if(_target_type_info == &object_type_info::null())
    {
        obj.clear();
        return;
    }

    for (const mixin_type_info* rem : _mutation._removing._compact_mixins)
    {
        // we allow removing of mixins that aren't even there
        if(obj.internal_has_mixin(rem->id))
            obj.destroy_mixin(rem->id);
    }

    obj.change_type(_target_type_info, false);

    for (const mixin_type_info* add : _mutation._adding._compact_mixins)
    {
        // we allow adding mixins that are already there
        if(!obj.internal_get_mixin(add->id))
            obj.construct_mixin(add->id);
    }
}

bool object_mutator::add(const char* mixin_type_name)
{
    mixin_id id = domain::instance().get_mixin_id_by_name(mixin_type_name);

    if(id == INVALID_MIXIN_ID)
    {
        return false;
    }

    _mutation.start_adding(id);

    return true;
}

bool object_mutator::remove(const char* mixin_type_name)
{
    mixin_id id = domain::instance().get_mixin_id_by_name(mixin_type_name);

    if(id == INVALID_MIXIN_ID)
    {
        return false;
    }

    _mutation.start_removing(id);

    return true;
}

void object_mutator::add(mixin_id id)
{
    DYNAMIX_THROW_UNLESS(id < domain::instance()._num_registered_mixins, bad_mutation);
    _mutation.start_adding(id);
}

void object_mutator::remove(mixin_id id)
{
    DYNAMIX_THROW_UNLESS(id < domain::instance()._num_registered_mixins, bad_mutation);
    _mutation.start_removing(id);
}

} // namespace internal
} // namespace dynamix
