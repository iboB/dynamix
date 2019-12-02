// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

/**
 * \file
 * Types related to mixin features.
 */

#include "config.hpp"

#include <cstddef>

namespace dynamix
{

typedef size_t feature_id;

// TODO: inline when on C++17
static constexpr feature_id INVALID_FEATURE_ID = ~feature_id(0);

class DYNAMIX_API feature
{
public:
    feature_id id;
    const char* const name;

    bool is_private; // true when it's never used outside a single module
    // registrators will register features of the same name as the same feature
    // if they're not private
    // having different features with the same name in different modules
    // may cause dangerous crashes if they're not private

protected:
    feature(const char* name, bool is_private)
        : id(INVALID_FEATURE_ID)
        , name(name)
        , is_private(is_private)
    {
    }

    // non-copyable
    feature(const feature&) = delete;
    feature& operator=(const feature&) = delete;
};


namespace internal
{

// like the mixin_type_info_instance this class is as a
// metafunction to bind feature types to their unique instances
template <typename Feature>
struct feature_instance
{
    // using a static function instead of a member to guarantee the constructor is called
    // however this leads to the compiler generating thread-safe initialization
    // which we don't care about
    static Feature& the_feature_safe()
    {
        static Feature f;
        return f;
    }

    // to work around the thread-safe slownes from the above function we can use this instead
    // HOWEVER, it's not global-call safe, so be sure to only call it when you're sure you're
    // not in global scope. Otherwise it may just return null
    static const Feature& the_feature_fast()
    {
        return _feature;
    }

    // unfortunately we cannot do the mixin trick of global instantiation here
    // there is no guarantee that the features will be instantiated before the mixins
    // and their id's are needed
    // instead we'll register the features manually when registering the mixin
    // this will be at the cost of having features registered multiple times

private:
    // used for the fast getter
    static const Feature& _feature;
};

template <typename Feature>
const Feature& feature_instance<Feature>::_feature = feature_instance::the_feature_safe();

} // namespace internal
} // namespace dynamix
