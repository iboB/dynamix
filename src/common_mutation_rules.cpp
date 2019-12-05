// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include <dynamix/common_mutation_rules.hpp>
#include <dynamix/object_type_mutation.hpp>
#include "dynamix/internal/assert.hpp"

namespace dynamix
{

using namespace internal;

void mutually_exclusive_mixins::apply_to(object_type_mutation& mutation)
{
    // find if the mutation is adding any of the mutually exclusive mixins

    mixin_id adding = INVALID_MIXIN_ID;

    for (const mixin_type_info* mixin_info : _compact_mixins)
    {
        if(mutation.is_adding(mixin_info->id))
        {
            I_DYNAMIX_ASSERT_MSG(adding == INVALID_MIXIN_ID, "mutation breaking a mutually exclusive mixin rule");
            adding = mixin_info->id;
#if !DYNAMIX_DEBUG
            // in debug mode continue the loop, to check for other rule breaking
            // otherwise break for faster execution
            break;
#endif
        }
    }

    //
    // what do we do if more than one of the mutually exclusive mixins is being added???
    //

    if(adding == INVALID_MIXIN_ID)
    {
        // nothing to do here
        return;
    }

    // find if the object has one of the mutually exclusive mixins
    for (const mixin_type_info* mixin_info : _compact_mixins)
    {
        if(mixin_info->id == adding)
        {
            continue;
        }

        if(mutation.source_has(mixin_info->id))
        {
            // remove all others from the object
            mutation.start_removing(mixin_info->id);
        }
    }
}

void bundled_mixins::apply_to(object_type_mutation& mutation)
{
    // find if the mutation is adding any of the bundled mixins

    mixin_id adding = INVALID_MIXIN_ID;
    mixin_id removing = INVALID_MIXIN_ID;

    for (const auto* mixin_info : _compact_mixins)
    {
        if (mutation.is_adding(mixin_info->id))
        {
            adding = mixin_info->id;
            break;
        }
        else if (mutation.is_removing(mixin_info->id))
        {
            removing = mixin_info->id;
            break;
        }
    }

    I_DYNAMIX_ASSERT_MSG(adding == INVALID_MIXIN_ID || removing == INVALID_MIXIN_ID, "mutation breaking a bundle mixin rule");

    if (adding != INVALID_MIXIN_ID)
    {
        for (const auto* mixin_info : _compact_mixins)
        {
            // add all others to the object
            mutation.start_adding(mixin_info->id);
        }
    }
    else if (removing != INVALID_MIXIN_ID)
    {
        for (const auto* mixin_info : _compact_mixins)
        {
            // remove all others from the object
            mutation.start_removing(mixin_info->id);
        }
    }
}

void dependent_mixins::apply_to(object_type_mutation& mutation)
{
    if (mutation.is_adding(_master_id))
    {
        for (const auto* mixin_info : _compact_mixins)
        {
            // add all dependent mixins to the object
            mutation.start_adding(mixin_info->id);
        }
    }
    else if (mutation.is_removing(_master_id))
    {
        for (const auto* mixin_info : _compact_mixins)
        {
            // remove all dependent mixins from the object
            mutation.start_removing(mixin_info->id);
        }
    }
}

namespace internal
{
void mandatory_mixin_impl::apply_to(object_type_mutation& mutation)
{
    if(mutation.is_removing(_id))
    {
        mutation.stop_removing(_id);
    }

    if(mutation.source_has(_id))
        return;

    mutation.start_adding(_id);
}

void deprecated_mixin_impl::apply_to(object_type_mutation& mutation)
{
    if(mutation.source_has(_id))
    {
        mutation.start_removing(_id);
    }

    if(mutation.is_adding(_id))
    {
        mutation.stop_adding(_id);
    }
}

void substitute_mixin_impl::apply_to(object_type_mutation& mutation)
{
    if(mutation.is_adding(_source_id))
    {
        mutation.stop_adding(_source_id);
        mutation.start_adding(_target_id);
    }
}

} // namespace internal
} // namespace dynamix
