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
 * The library's exceptions.
 */

#include "global.hpp"
#include "assert.hpp"

#if DYNAMIX_USE_EXCEPTIONS

namespace dynamix
{

/// Parent class of all DynaMix exceptions.
class exception : public ::std::exception {};

/// Thrown when an attempt is made to mutate an object with an incomplete mutation
///
/// Currently the only way you could have an incomplete mutation is trying to
/// apply an `object_type_template` for which `create` hasn't been called.
class bad_mutation : public exception {};

/// Thrown when an attempt is made to mutate an object whose type doesn't correspond
/// to the expected source type of the mutation (the one to mutate *from*).
///
/// The most likely cause is when using a `same_type_mutator` on objects that are
/// not of the same type
class bad_mutation_source : public exception {};

/// Thrown when a message is called on a object that doesn't implement it.
class bad_message_call : public exception {};

/// Thrown when an object type is created which has mixins that implement
/// the same unicast message with the same priority
class unicast_clash : public exception {};

/// Thrown when a copy construction is performed from an object which has a
/// non-copy-constructioble mixin
class bad_copy_construction : public exception {};

/// Thrown when a copy assignment is performed from an object which has a
/// non-copy-assignable mixin
class bad_copy_assignment : public exception {};

}

/// A macro that throws an exception if `DYNAMIX_USE_EXCEPTIONS`
/// is defined. If it's not, it falls back to an assert.
#   define DYNAMIX_THROW_UNLESS(test, exception) do { if(!(test)) { throw exception(); } } while(false)

#else // DYNAMIX_USE_EXCEPTIONS

#   define DYNAMIX_THROW_UNLESS(test, exception) DYNAMIX_ASSERT_MSG(test, _DYNAMIX_PP_STRINGIZE(exception))

#endif // DYNAMIX_USE_EXCEPTIONS


#if !defined(DYNAMIX_NO_MSG_THROW)
/// This macro is used internally to throw exceptions within the functions generated
/// for the mixin messages.
///
/// \note You can disable the throwing of exceptions in the message functions by defining
///  `DYNAMIX_NO_MSG_THROW` before including DynaMix's headers. If you do this,
/// the messages will fall back to having `assert` calls in them
#   define DYNAMIX_MSG_THROW_UNLESS(test, exception) DYNAMIX_THROW_UNLESS(test, exception)
#else
#   define DYNAMIX_MSG_THROW_UNLESS(test, exception) DYNAMIX_ASSERT_MSG(test, _DYNAMIX_PP_STRINGIZE(exception))
#endif
