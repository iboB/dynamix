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
 * Macros to declare mixins
 */

#include "config.hpp"
#include "internal/preprocessor.hpp"

namespace dynamix
{
class mixin_type_info;
}

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
    extern export ::dynamix::mixin_type_info& _dynamix_get_mixin_type_info(const mixin_type* m)

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
    DYNAMIX_DECLARE_EXPORTED_MIXIN(I_DYNAMIX_PP_EMPTY(), mixin_type)
