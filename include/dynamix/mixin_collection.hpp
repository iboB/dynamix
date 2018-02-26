// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "global.hpp"
#include "mixin_type_info.hpp"

namespace dynamix
{

/// A mixin collection is a class that allows the user to
/// represent a number of mixins
class DYNAMIX_API mixin_collection
{
public:
    mixin_collection();
    mixin_collection(const internal::mixin_type_info_vector& mixins);

    // non-copyable
    mixin_collection(const mixin_collection&) = delete;
    mixin_collection& operator=(const mixin_collection&) = delete;

    /// Checks if a mixin type is present in the collection
    template <typename Mixin>
    bool has() const
    {
        const internal::mixin_type_info& info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        return has(info.id);
    }
    bool has(mixin_id id) const { return _mixins[id]; }

    /// Adds a mixin type is to the collection
    template <typename Mixin>
    void add()
    {
        const internal::mixin_type_info& info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        check_valid_mixin(info);
        add(info.id);
    }
    void add(mixin_id id);

    /// Removes a mixin type from the collection
    template <typename Mixin>
    void remove()
    {
        const internal::mixin_type_info& info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        remove(info.id);
    }
    void remove(mixin_id id);

    /// Checks if a feature is implemented by any mixin in the collection
    template <typename Feature>
    bool implements(const Feature* f) const;

    /// Clears all mixins implementing a feature from the collection
    template <typename Feature>
    void clear_all_implementing(const Feature* f);

    internal::available_mixins_bitset _mixins;
    // only the mixins the objects of this type have
    internal::mixin_type_info_vector _compact_mixins;

    void rebuild_from_compact_mixins();
    void check_valid_mixin(const internal::mixin_type_info& mixin_info);

    /// Removes all elements from the collection
    void clear();

    /// Checks if a collection is empty.
    bool empty() const { return _compact_mixins.empty(); }
};


} // namespace dynamix
