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
 * Functions and macros associated with mixin definition
 */

#include "config.hpp"
#include "domain.hpp"
#include "mixin_type_info.hpp"
#include "features.hpp"
#include "internal/feature_parser.hpp"
#include "internal/mixin_traits.hpp"

namespace dynamix
{
namespace internal
{

// this metafunction binds the type info of a mixin to its type
template <typename Mixin>
struct mixin_type_info_instance
{
    // have this static function instead of a simple member to guarantee
    // that mixin_type_info's constructor is called the first time
    static mixin_type_info& info()
    {
        static mixin_type_info d;
        return d;
    }

    // this static member registers the mixin in the domain
    // we need to reference it somewhere so as to call its constructor
    static mixin_type_info_instance registrator;

    mixin_type_info_instance()
    {
        auto& the_info = info();

        feature_parser_phase_1 p1(the_info);
        _dynamix_parse_mixin_features(static_cast<Mixin*>(nullptr), p1);

        the_info.message_infos.reserve(p1.num_messages());

        feature_parser_phase_2<Mixin> p2(the_info);
        _dynamix_parse_mixin_features(static_cast<Mixin*>(nullptr), p2);

        set_missing_traits_to_info<Mixin>(the_info);

        // register the mixin in the domain
        domain::safe_instance().register_mixin_type(the_info);
    }


    ~mixin_type_info_instance()
    {
        // unregister the mixin from the domain
        domain::safe_instance().unregister_mixin_type(info());
    }


    // non-copyable
    mixin_type_info_instance(const mixin_type_info_instance&) = delete;
    mixin_type_info_instance& operator=(const mixin_type_info_instance&) = delete;

    // to prevent warnings and optimizations that will say that we're not using
    // mixin_type_info_instance by simply referencing it
    int unused;
};
template <typename Mixin>
mixin_type_info_instance<Mixin> mixin_type_info_instance<Mixin>::registrator;

} // namespace internal

#if DYNAMIX_USE_MIXIN_NAME_FROM_MACRO
#   define I_DYNAMIX_MIXIN_NAME_FEATURE ::dynamix::mixin_name
#else
#   define I_DYNAMIX_MIXIN_NAME_FEATURE ::dynamix::noop_feature
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
