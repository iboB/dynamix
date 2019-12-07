#pragma once

//
// dynamix.hpp - main include file for the DynaMix library
//
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//

/** @file
 * The main include file of the library.
 * Includes all required files
 * (Note: also including some that could become optional)
 */

#if defined(_MSC_VER)
// don't leave our warning disabled for the outside world
#   pragma warning( push )
#endif

#include "config.hpp"
#include "domain.hpp"
#include "feature.hpp"
#include "features.hpp"
#include "mixin_type_info.hpp"
#include "declare_mixin.hpp"
#include "define_mixin.hpp"
#include "object_of.hpp"
#include "message.hpp"
#include "declare_message_opt.hpp"
#include "define_message.hpp"
#include "object.hpp"
#include "single_object_mutator.hpp"
#include "same_type_mutator.hpp"
#include "object_type_template.hpp"
#include "object_type_info.hpp"
#include "exception.hpp"
#include "allocators.hpp"

#if !defined(DYNAMIX_NO_DM_THIS)
#   include "dm_this.hpp"
#endif

// possibly leave those to be included separately ?
#include "next_bidder.hpp"
#include "mutation_rule.hpp"
#include "common_mutation_rules.hpp"
#include "combinators.hpp"

#if defined(_MSC_VER)
#   pragma warning( pop )
#endif
