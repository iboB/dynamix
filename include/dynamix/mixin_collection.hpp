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

#include <bitset>
#include <vector>

namespace dynamix
{

namespace internal
{
using mixin_type_info_vector = std::vector<const mixin_type_info*>;
using available_mixins_bitset = std::bitset<DYNAMIX_MAX_MIXINS>;
} // namespace internal

/// A mixin collection is a class that allows the user to
/// represent a number of mixins
class DYNAMIX_API mixin_collection
{
public:
    mixin_collection() noexcept;
    mixin_collection(const internal::mixin_type_info_vector& mixins);

    ~mixin_collection();

    // non-copyable
    mixin_collection(const mixin_collection&) = delete;
    mixin_collection& operator=(const mixin_collection&) = delete;

    /// Checks if a mixin type is present in the collection
    template <typename Mixin>
    bool has() const noexcept
    {
        const mixin_type_info& info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        return has(info.id);
    }
    bool has(const mixin_type_info& info) const noexcept { return has(info.id); }
    bool has(mixin_id id) const noexcept { return _mixins[id]; }

    /// Adds a mixin type is to the collection
    template <typename Mixin>
    void add()
    {
        const mixin_type_info& info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        add(info);
    }
    void add(const mixin_type_info& info);
    void add(mixin_id id);

    /// Removes a mixin type from the collection. Returns whether something was removed
    template <typename Mixin>
    bool remove()
    {
        const mixin_type_info& info = _dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr));
        return remove(info);
    }
    bool remove(const mixin_type_info& info);
    bool remove(mixin_id id);

    /// Removes all elements from the collection
    void clear();

    /// Checks if a collection is empty.
    bool empty() const { return _compact_mixins.empty(); }

    void rebuild_from_compact_mixins();
    internal::available_mixins_bitset _mixins;
    internal::mixin_type_info_vector _compact_mixins;
};

} // namespace dynamix
