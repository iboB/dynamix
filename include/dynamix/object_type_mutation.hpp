// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
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
class DYNAMIX_API object_type_mutation
{
public:
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

    bool is_adding(mixin_id id) const
    {
        return _adding.has(id);
    }
    bool is_removing(mixin_id id) const
    {
        return _removing.has(id);
    }

    bool is_adding(const mixin_type_info& info) const
    {
        return _adding.has(info);
    }
    bool is_removing(const mixin_type_info& info) const
    {
        return _removing.has(info);
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

    void stop_adding(const mixin_type_info& info)
    {
        _adding.remove(info);
    }
    void stop_removing(const mixin_type_info& info)
    {
        _removing.remove(info);
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

    void start_adding(const mixin_type_info& info)
    {
        _adding.add(info);
    }
    void start_removing(const mixin_type_info& info)
    {
        _removing.add(info);
    }

    /// Returns true if the mutation is empty - adds no mixins and removes no mixins.
    bool empty() const { return _adding.empty() && _removing.empty(); }

    /// Normalize the collections _adding and _removing.
    /// That is, if an element is in both, it will be removed from both.
    void normalize();

    /// Clears a mutation. Makes it a noop - adds no mixins and removes no mixins.
    void clear()
    {
        _adding.clear();
        _removing.clear();
    }

private:
    friend class internal::object_mutator;

    mixin_collection _adding;
    mixin_collection _removing;
};

} // namespace dynamix
