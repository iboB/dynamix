// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

/**
 * \file
 * Defines global internal functions, classes, and type definitions.
 */

#include "config.hpp"

#include <algorithm>
#include <functional> // for std::greater
#include <vector>
#include <cstring> // for memset

// logically internal data within classes that cannot be private or protected
// due to implementation issues is marked with _dynamix_internal
// class X
// {
// public:
//    <public stuff>
// _dynamix_internal:
//    <actually public but logically private stuff>
// };
#define _dynamix_internal public


#include <bitset>

namespace dynamix
{
    typedef size_t mixin_id;
    typedef size_t feature_id;
    typedef size_t mutation_rule_id;

namespace internal
{
    // simple and often used function that just checks if an element is present
    // in a forward-iteratable container
    template <typename Container, typename Elem>
    bool has_elem(const Container& c, const Elem& e)
    {
        return std::find(c.begin(), c.end(), e) != c.end();
    }

    template <typename T>
    bool is_sorted(const std::vector<T>& v)
    {
        return std::adjacent_find(v.begin(), v.end(), std::greater<T>()) == v.end();
    }

    inline void zero_memory(void* mem, size_t size)
    {
        std::memset(mem, 0, size);
    }

    class mixin_type_info;
    typedef std::vector<const mixin_type_info*> mixin_type_info_vector;

    typedef std::bitset<DYNAMIX_MAX_MIXINS> available_mixins_bitset;

    extern DYNAMIX_API available_mixins_bitset build_available_mixins_from(const mixin_type_info_vector& mixins);

} // namespace internal
} // namespace dynamix
