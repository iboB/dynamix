// DynaMix
// Copyright (c) 2013-2020 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once


/**
 * \file
 * Defines an internal base class, that is a parent to various concrete types
 * that deal with object mutation.
 */

#include "config.hpp"
#include "object_type_mutation.hpp"
#include "internal/assert.hpp"

namespace dynamix
{

class object;
class object_type_info;

namespace internal
{

class DYNAMIX_API object_mutator
{
public:
    object_mutator();
    object_mutator(const mixin_collection* source_mixins);

    // non-copyable
    object_mutator(const object_mutator&) = delete;
    object_mutator& operator=(const object_mutator&) = delete;

    template <typename Mixin>
    object_mutator& add()
    {
        I_DYNAMIX_ASSERT(!_is_created); // once a mutator is created, you cannot change its mutation
        _mutation.start_adding<Mixin>();
        return *this;
    }

    template <typename Mixin>
    object_mutator& remove()
    {
        I_DYNAMIX_ASSERT(!_is_created); // once a mutator is created, you cannot change its mutation
        _mutation.start_removing<Mixin>();
        return *this;
    }

    // add/remove mixin by type name string
    // both functions will return whether a mixin with this name exists
    // Note: the return value has nothing to with whether the mixin has been added
    // or removed from the object(s) being mutated (this is resolved by the mutation rules)
    bool add(const char* mixin_type_name);
    bool remove(const char* mixin_type_name);

    // add/remove mixin by mixin id
    // ann exception is thrown if a mixin with this id doesn't exist
    void add(mixin_id id);
    void remove(mixin_id id);

    // todo: add operators + and -

    void cancel();

    void create();

    const object_type_mutation& mutation() const { return _mutation; }

protected:
    void apply_to(object& obj) const;

    object_type_mutation _mutation;
    const mixin_collection* _source_mixins = nullptr; // mixins the object being mutated
    const object_type_info* _target_type_info = nullptr; // new type info of the object

    bool _is_created = false;
};

} // namespace internal
} // namespace dynamix
