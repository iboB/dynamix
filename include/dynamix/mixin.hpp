// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
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

#include "global.hpp"
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
        template register_mixin_type<Mixin>(_dynamix_get_mixin_type_info((Mixin*)nullptr));
}

template <typename Mixin>
mixin_type_info_instance<Mixin>::~mixin_type_info_instance()
{
    // unregister the mixin from the domain
    domain::safe_instance().unregister_mixin_type(info());
}

} // namespace internal


/**
 * \brief Declares a dynamic library class as a mixin.
 *
 * \param export is the type of export. For example `__declspec(dllexport)`
 * in Visual C++.
 * \param mixin_type is the class name of the declared mixin.
 *
 * When using mixins from dynamic libraries, use this macro in your headers
 * to forward declare a class as a mixin.
 * This may be included separately and doesn't need to be in the same
 * header as the actual mixin class definition.
 *
 * If the first parameter is empty, the macro is equivalent to
 * DYNAMIX_DECLARE_MIXIN
 *
 * \par Example:
 * \code
 * // Assuming MY_LIB_API is a macro that expands accordingly to the
 * // export/import symbols for the compiler you're using.
 * DYNAMIX_DECLARE_EXPORTED_MIXIN(MY_LIB_API, my_mixin_type);
 * \endcode
 */
#define DYNAMIX_DECLARE_EXPORTED_MIXIN(export, mixin_type) \
    class mixin_type; \
    extern export ::dynamix::internal::mixin_type_info& _dynamix_get_mixin_type_info(const mixin_type* m)

/**
 * \brief Declares a class as a mixin.
 *
 * \param mixin_type is the class name of the declared mixin.
 *
 * Call this in header files to forward declare a mixin type.
 * This may be included separately and doesn't need to be in the same
 * header as the actual mixin class definition.
 *
 */
#define DYNAMIX_DECLARE_MIXIN(mixin_type) \
    DYNAMIX_DECLARE_EXPORTED_MIXIN(_DYNAMIX_PP_EMPTY(), mixin_type)

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
    ::dynamix::internal::mixin_type_info& _dynamix_get_mixin_type_info(const mixin_type*) { return ::dynamix::internal::mixin_type_info_instance<mixin_type>::info(); } \
    /* create a features parsing function */ \
    /* features can be parsed multiple times by different parsers */ \
    template <typename FeaturesParser> \
    void _dynamix_parse_mixin_features(const mixin_type*, FeaturesParser& parser) { parser & mixin_features; }

class object;

/**
 * \brief gets the object of a mixin
 *
 * \param[in] mixin_addr the address of the mixin
 *
 * \return A pointer to the object of the given mixin
 *
 * Returns the owning object of a given mixin.
 * \warning This function just makes a pointer offset and cast.
    It will work with any object that's been given to it
    without a warning or an error. Even, say `int*`. It is a
    source of potential bugs if you don't make sure that the
    input pointer is a mixin, that is a part of an object
 *
 * \par Example:
 * \code
 * mymixin* ptr = myobject->get<mymixin>();
 * object_of(ptr); // == myobject
 * \endcode
 *
 * \see #dm_this
 */
template <typename Mixin>
object* object_of(Mixin* mixin_addr)
{
    return *reinterpret_cast<object**>(reinterpret_cast<char*>(mixin_addr) - sizeof(object*));
}

/**
 * \copydoc object_of()
*/
template <typename Mixin>
const object* object_of(const Mixin* mixin_addr)
{
    return *reinterpret_cast<const object*const*>(reinterpret_cast<const char*>(mixin_addr) - sizeof(object*));
}

} // namespace dynamix

// this macro makes writing code within mixins nicer
#if !defined(DYNAMIX_NO_DM_THIS)
/**
 * \brief a pointer to the owning object of the current mixin
 *
 * Much like `this` is a pointer to the current class, `dm_this`
 * is a macro that, for mixins, points to the current object.
 *
 * It is nothing more than `dynamix::object_of(this)`
 *
 * \note You can disable the definition of this macro by
 * defining `DYNAMIX_NO_DM_THIS` before including the
 * library's headers.
 *
 * \see object_of()
*/
#   define dm_this ::dynamix::object_of(this)
#endif
