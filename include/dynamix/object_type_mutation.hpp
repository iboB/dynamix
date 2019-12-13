// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "config.hpp"
#include "mixin_type_info.hpp"
#include "mixin_collection.hpp"

namespace dynamix
{
namespace internal
{
    class object_mutator;
}

/// This class represents an object mutation.
/// It is used by mutators and mutation rules.
///
/// Internally the class has two `mixin_collection` objects -
/// `removing` and `adding`. They represent the mixins that are supposed to be
/// removed and added by the mutation.
///
/// Additionally another mixin collection might be present - the source.
/// It is a pointer that may be null. If it's not, it represents the mixins
/// of the object that currently being mutated by this mutation.
class DYNAMIX_API object_type_mutation
{
public:
    /// Constructs an empty mutation
    object_type_mutation();

    /// Constructs a mutation with a specific source.
    object_type_mutation(const mixin_collection* src);

    /// Sets the source of the mutation.
    void set_source(const mixin_collection* src) { _source = src; }

    /// Checks if the mutation is adding a mixin.
    template <typename Mixin>
    bool is_adding() const
    {
        return _adding.has<Mixin>();
    }
    /// Checks if the mutation is removing a mixin.
    template <typename Mixin>
    bool is_removing() const
    {
        return _removing.has<Mixin>();
    }
    /// Checks if the mutation's source has a mixin.
    template <typename Mixin>
    bool source_has() const
    {
        return _source->has<Mixin>();
    }

    bool is_adding(mixin_id id) const
    {
        return _adding.has(id);
    }
    bool is_removing(mixin_id id) const
    {
        return _removing.has(id);
    }
    bool source_has(mixin_id id) const
    {
        return _source->has(id);
    }

    /// Removes a mixin from the ones being added by the mutation.
    template <typename Mixin>
    void stop_adding()
    {
        _adding.remove<Mixin>();
    }
    /// Removes a mixin from the ones being removed by the mutation.
    template <typename Mixin>
    void stop_removing()
    {
        _removing.remove<Mixin>();
    }

    void stop_adding(mixin_id id)
    {
        _adding.remove(id);
    }
    void stop_removing(mixin_id id)
    {
        _removing.remove(id);
    }

    /// Adds a mixin to the ones being added by the mutation.
    template <typename Mixin>
    void start_adding()
    {
        _adding.add<Mixin>();
    }
    /// Adds a mixin to the ones being removed by the mutation.
    template <typename Mixin>
    void start_removing()
    {
        _removing.add<Mixin>();
    }

    void start_adding(mixin_id id)
    {
        _adding.add(id);
    }
    void start_removing(mixin_id id)
    {
        _removing.add(id);
    }

    /// Returns true if the mutation is empty - adds no mixins and removes no mixins.
    bool empty() const { return _adding.empty() && _removing.empty(); }

    /// Normalize the collections _adding and _removing.
    /// That is, if an element is in both, it will be removed from both.
    void normalize();

    /// Clears a mutation. Restores it to its initial state.
    void clear();

private:
    friend class internal::object_mutator;

    mixin_collection _adding;
    mixin_collection _removing;
    const mixin_collection* _source;
};

} // namespace dynamix
