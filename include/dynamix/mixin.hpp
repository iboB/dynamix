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
 * Functions and macros associated with mixin declaration,
 * definition, and usage.
 */

#include "config.hpp"
#include "domain.hpp"
#include "mixin_type_info.hpp"
#include "feature.hpp"
#include "preprocessor.hpp"

namespace dynamix
{

namespace internal
{

template <typename Mixin>
mixin_type_info_instance<Mixin>::mixin_type_info_instance()
{
    // register the mixin in the domain
    domain::safe_instance().
        // we use the function to get the type info, to guarantee that an instantiation of the template
        // from another module won't override if
        template register_mixin_type<Mixin>(_dynamix_get_mixin_type_info(static_cast<Mixin*>(nullptr)));
}

template <typename Mixin>
mixin_type_info_instance<Mixin>::~mixin_type_info_instance()
{
    // unregister the mixin from the domain
    domain::safe_instance().unregister_mixin_type(info());
}

} // namespace internal

#if DYNAMIX_USE_MIXIN_NAME_FROM_MACRO
#   define I_DYNAMIX_MIXIN_NAME_FEATURE ::dynamix::mixin_name
#else
#   define I_DYNAMIX_MIXIN_NAME_FEATURE ::dynamix::internal::noop_feature
#endif


/**
 * \brief defines a mixin
 *
 * \param mixin_type is the class name of the declared mixin.
 * \param mixin_features the mixin features
 *
 * The macro defines a mixin in the domain. Call this once per mixin
 * in a compilation unit (.cpp file). A good idea is, if possible, to place it
 * in the compilation unit of the mixin class itself.
 *
 * To work properly, the macro needs to see a forward declaration of the mixin
 * (by DYNAMIX_DECLARE_MIXIN or DYNAMIX_DECLARE_EXPORED_MIXIN).
 *
 * \par Mixin features:
 * The features argument is an ampersand (&) separated list of the features -
 * messages, allocators, and others - that the mixin will support. If the
 * features have a special suffix (as messages do), it applies here.
 *
 * If the mixin has no features, then `dynamix::none` should be set as
 * the last argument
 *
 * \par Example:
 * \code
 * DYNAMIX_DEFINE_MIXIN(mymixin, foo_msg & priority(1, bar_msg) & allocator<myalloc>);
 * DYNAMIX_DEFINE_MIXIN(simple_mixin, none);
 * \endcode
 */
#define DYNAMIX_DEFINE_MIXIN(mixin_type, mixin_features) \
    \
    /* create a function that will reference mixin_type_info_instance static registrator to guarantee its instantiation */ \
    inline void _dynamix_register_mixin(mixin_type*) { ::dynamix::internal::mixin_type_info_instance<mixin_type>::registrator.unused = true; } \
    /* create a mixin_type_info getter for this type */ \
    ::dynamix::mixin_type_info& _dynamix_get_mixin_type_info(const mixin_type*) { return ::dynamix::internal::mixin_type_info_instance<mixin_type>::info(); } \
    /* create a features parsing function */ \
    /* features can be parsed multiple times by different parsers */ \
    template <typename FeaturesParser> \
    void _dynamix_parse_mixin_features(const mixin_type*, FeaturesParser& parser) { parser & I_DYNAMIX_MIXIN_NAME_FEATURE(#mixin_type) & mixin_features; }

} // namespace dynamix
