// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
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

// custom config file which allows users to change the library configuration
// without changing this file
// this can be useful when using it as a submodule, or when the same source
// is used in several projects
// WARNING: be very careful with this! You cannot just change the configuration
// before including the library. If the configuration is changed, you MUST
// also rebuild it, and take care to use the same custom config for the library
// and all client modules
#if defined DYNAMIX_CUSTOM_CONFIG_FILE
#   include DYNAMIX_CUSTOM_CONFIG_FILE
#endif

// DynaMix heavily relies on type names
// setting the DYNAMIX_USE_TYPEID to true will cause it to obtain said names with type_info via typeid.
// setting it to false will require the following extra steps:
// if DYNAMIX_USE_MIXIN_NAME_FROM_MACRO
//      mixins will obtain the name from the macro
//      WARNING: multiple with the same name in different namespaces won't work!
// if DYNAMIX_USE_STATIC_MEMBER_NAME is defined
//      mixins must add static const char* dynamix_mixin_name() { return <mixin class name>; }
// if none of the above is defined
//      users must supply the name of the mixins with the mixin_name feature
#if !defined(DYNAMIX_USE_TYPEID)
#   define DYNAMIX_USE_TYPEID 1
#endif

#if !DYNAMIX_USE_TYPEID
// backwards compatibility
// in previous versions when typeid was not used to get the mixin name
// the user had to set a manual name for a mixin by providing a
// method `static const char* dynamix_mixin_name` per mixin
// this is deprecated but kept for backwards compatibility
#   if !defined(DYNAMIX_USE_STATIC_MEMBER_NAME)
#       define DYNAMIX_USE_STATIC_MEMBER_NAME 0
// an alternative way of obtaining mixin names without typeid is to infer them
// from the mixin definition macro
// HOWEVER if you have mixins in namespaces the namespace name won't be included
// this may lead to name clasesh if two mixins with the same name exist in
// different namespaces.
// USE WITH CAUTION
#   elif !defined(DYNAMIX_USE_MIXIN_NAME_FROM_MACRO)
#       define DYNAMIX_USE_MIXIN_NAME_FROM_MACRO 0
#   endif
#endif

// enable various debug checks and assertions
#if !defined(NDEBUG) && !defined(DYNAMIX_DEBUG)
#   define DYNAMIX_DEBUG 1
#endif

// maximum number of registered mixins
// an assertion will occur if this limit is reached in a program
// *    the object type creation and the object mutation are proportional
//      to this value by a factor of o(n log(word, n)) where word is the bit size of size_t
// *    the object type's size is 2 * <value> / 8 (a bit for each possible mixin)
//      the more global object of different types you have the more type memory they'll consume
// ie: minor cost for increase
#if !defined(DYNAMIX_MAX_MIXINS)
#   define DYNAMIX_MAX_MIXINS 512
#endif

// maximum number of messages (and consequently max per mixin)
// an assertion will occur if this limit is reached in a program
// as with the mixin limits this influences object type with 2 times as many pointers (2 * <word> * value)
// object creation and mutation is not influenced by this value
// minor cost for increase
#if !defined(DYNAMIX_MAX_MESSAGES)
#   define DYNAMIX_MAX_MESSAGES 1024
#endif

// setting this to true will cause some functions to throw exceptions instead of asserting
#if !defined(DYNAMIX_USE_EXCEPTIONS)
#   define DYNAMIX_USE_EXCEPTIONS 1
#endif

// setting this to true will cause the object type to have a copy constructor and a copy assignment operator
// however it is set to false by default so as to prevent accidental copying of objects
// explicit copying is still possible via object::copy_from or object::copy
#if !defined(DYNAMIX_OBJECT_IMPLICIT_COPY)
#   define DYNAMIX_OBJECT_IMPLICIT_COPY 0
#endif

// setting this to true will add a mutex for the object type infos and lock it when new type infos
// are created queried or created, thus making the mutation of multiple objects in multiple threads
// safe.
// HOWEVER
// mutating the same object in multiple threads is never safe
// mutating an object in one thread and calling messages for this object in another is never safe
#if !defined(DYNAMIX_THREAD_SAFE_MUTATIONS)
#   define DYNAMIX_THREAD_SAFE_MUTATIONS 1
#endif

// setting this to true will add additional metrics to the library's structures
// maintaining them is cheap, but does not cost zero
// the additional metrics are
// mixin_type_info::num_mixins - number of living mixins of this type
#if !defined(DYNAMIX_ADDITIONAL_METRICS)
#   define DYNAMIX_ADDITIONAL_METRICS 1
#endif

// setting this to true will enable the compilation of object::replace_mixin and object::move_mixin
// they can be dangerous as clients which keep pointers to mixins within objects can have them
// invalidated without a way to be notified about this
// however those functions can be quite useful for complex allocation strategies which try to
// keep some mixins in a congiguous block of memory
#if !defined(DYNAMIX_OBJECT_REPLACE_MIXIN)
#   define DYNAMIX_OBJECT_REPLACE_MIXIN 1
#endif

// there is warning push/pop about this in the main header
#if defined(_MSC_VER)
// msvc complains that template classes don't have a dll interface (they shouldn't).
#   pragma warning (disable: 4251)
#endif

// dynamic library interface
#if defined(DYNAMIX_DYNLIB)

// dynamic link macros
#   if defined(_MSC_VER)
#       define DYNAMIX_SYMBOL_EXPORT __declspec(dllexport)
#       define DYNAMIX_SYMBOL_IMPORT __declspec(dllimport)
#   else
#       define DYNAMIX_SYMBOL_EXPORT __attribute__((__visibility__("default")))
#       define DYNAMIX_SYMBOL_IMPORT __attribute__((__visibility__("default")))
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
