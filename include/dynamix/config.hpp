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
 * The library's configuration file.
 * Changing anything here will require rebuilding the library
 */

// DynaMix heavily relies on type names
// setting the DYNAMIX_USE_TYPEID to true will cause it to obtain said names with type_info via typeid.
// setting it to false will require the following extra steps:
//     * mixins must add static const char* dynamix_mixin_name() { return <mixin class name>; }
#define DYNAMIX_USE_TYPEID 1

#if !defined(NDEBUG)
#   define DYNAMIX_DEBUG // define this for ease of use
#endif

// maximum number of registered mixins
// an assertion will occur if this limit is reached in a program
// *    the object type creation and the object mutation are proportional
//      to this value by a factor of o(n log(word, n)) where word is the bit size of size_t
// *    the object type's size is 2 * <value> / 8 (a bit for each possible mixin)
//      the more global object of different types you have the more type memory they'll consume
// ie: minor cost for increase
#define DYNAMIX_MAX_MIXINS 512

// maximum number of messages (and consequently max per mixin)
// an assertion will occur if this limit is reached in a program
// as with the mixin limits this influences object type with 2 times as many pointers (2 * <word> * value)
// object creation and mutation is not influenced by this value
// minor cost for increase
#define DYNAMIX_MAX_MESSAGES 1024

// setting this to true will cause some functions to throw exceptions instead of asserting
#define DYNAMIX_USE_EXCEPTIONS 1

// there is warning push/pop about this in the main header
#if defined(_MSC_VER)
// msvc complains that template classes don't have a dll interface (they shouldn't).
#   pragma warning (disable: 4251)
#endif

#if defined(DYNAMIX_DYNLIB)

// dynamic link macros
#   if defined(_MSC_VER)
#       define DYNAMIX_SYMBOL_EXPORT __declspec(dllexport)
#       define DYNAMIX_SYMBOL_IMPORT __declspec(dllimport)
#   else
#       define DYNAMIX_SYMBOL_EXPORT __attribute__((__visibility__("default")))
#       define DYNAMIX_SYMBOL_IMPORT
    // other compilers ?
#   endif

#   if defined(DYNAMIX_SRC)
#       define DYNAMIX_API DYNAMIX_SYMBOL_EXPORT
#   else
#       define DYNAMIX_API DYNAMIX_SYMBOL_IMPORT
#   endif
#endif

#if !defined(DYNAMIX_API)
#   define DYNAMIX_API // static lib, so nothing
#endif
