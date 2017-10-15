// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include <dynamix/domain.hpp>
#include <dynamix/mutation_rule.hpp>
#include <dynamix/allocators.hpp>

namespace dynamix
{

no_features_t* none;

namespace internal
{

DYNAMIX_API default_allocator the_default_allocator;

domain& domain::safe_instance()
{
    static domain the_domain;
    return the_domain;
}

const domain& domain::_instance = domain::safe_instance();

const domain& domain::instance()
{
    return _instance;
}

domain::domain()
    : _num_registered_mixins(0)
    , _num_registered_messages(0)
    , _allocator(&the_default_allocator)
{
    zero_memory(_mixin_type_infos, sizeof(_mixin_type_infos));
    zero_memory(_messages, sizeof(_messages));
}

domain::~domain()
{
    for(auto& i : _object_type_infos)
    {
        delete i.second;
    }
}

mutation_rule_id domain::add_mutation_rule(mutation_rule* rule)
{
    std::shared_ptr<mutation_rule> ptr(rule);
    return add_mutation_rule(ptr);
}

mutation_rule_id domain::add_mutation_rule(std::shared_ptr<mutation_rule> rule)
{
#if DYNAMIX_THREAD_SAFE_MUTATIONS
    std::lock_guard<std::mutex> lock(_mutation_rules_mutex);
#endif

    // find free slot
    for (mutation_rule_id i = 0; i < _mutation_rules.size(); ++i)
    {
        auto& r = _mutation_rules[i];
        if (!r)
        {
            r = rule;
            return i;
        }
    }

    _mutation_rules.emplace_back(std::move(rule));
    return _mutation_rules.size() - 1;
}

std::shared_ptr<mutation_rule> domain::remove_mutation_rule(mutation_rule_id id)
{
#if DYNAMIX_THREAD_SAFE_MUTATIONS
    std::lock_guard<std::mutex> lock(_mutation_rules_mutex);
#endif

    if (id >= _mutation_rules.size()) return std::shared_ptr<mutation_rule>();

    auto ret = _mutation_rules[id];
    _mutation_rules[id].reset();
    return ret;
}

void domain::apply_mutation_rules(object_type_mutation& mutation)
{
#if DYNAMIX_THREAD_SAFE_MUTATIONS
    std::lock_guard<std::mutex> lock(_mutation_rules_mutex);
#endif

    for (auto& rule : _mutation_rules)
    {
        if (rule)
        {
            rule->apply_to(mutation);
        }
    }
}

const object_type_info* domain::get_object_type_info(const mixin_type_info_vector& mixins)
{
    // the mixin type infos need to be sorted
    // so as to guarantee that two object type infos of the same mixins
    // will have the exact same content
    DYNAMIX_ASSERT(is_sorted(mixins));

    available_mixins_bitset query;

    for(const mixin_type_info* info : mixins)
    {
        query[info->id] = true;
    }

#if DYNAMIX_THREAD_SAFE_MUTATIONS
    // TODO C++17: do this with a shared mutex instead
    std::lock_guard<std::mutex> lock(_object_type_infos_mutex);
#endif

    object_type_info_map::iterator i = _object_type_infos.find(query);

    if(i != _object_type_infos.end())
    {
        // get existing
        DYNAMIX_ASSERT(mixins == i->second->_compact_mixins);
        return i->second;
    }
    else
    {
        // create object type info
        // use unique_ptr since fill_call_table might throw
        std::unique_ptr<object_type_info> new_type(new object_type_info);
        new_type->_compact_mixins = mixins;

        for(size_t i=0; i<mixins.size(); ++i)
        {
            DYNAMIX_ASSERT(mixins[i]);
            new_type->_mixins[mixins[i]->id] = true;
            new_type->_mixin_indices[mixins[i]->id] = i + object_type_info::MIXIN_INDEX_OFFSET;
        }

        new_type->fill_call_table();

        _object_type_infos.insert(make_pair(query, new_type.get()));
        return new_type.release();
    }
}

void domain::internal_register_feature(message_t& m)
{
    // we need to see if we don't already have a message by that name
    // if we do we'll set this instantiation's id to the already existing one
    // and disregard this instantiation
    //
    // HERE IT GETS DANGEROUS
    // there is a possiblity to have messages of the same name, but for totally different methods
    // if this happened in the same module, the linker wouldn't allow it
    // the PRIVATE_MESSAGE macro will define the messages that are used within a single module
    // however if one misses it
    // and there happen to be messages of the same name, but for different methids,
    // registered within a single domain from different modules
    // crashes may ensue (as a message gets called, for objects that can't actually handle it)

    feature_id free = INVALID_FEATURE_ID;

    // check for message of the same name
    for(size_t i=0; i<_num_registered_messages; ++i)
    {
        if (!_messages[i])
        {
            free = i;
            continue;
        }

        message_t& registered_message = *_messages[i];

        DYNAMIX_ASSERT(registered_message.id != INVALID_FEATURE_ID); // how could this happen?

        if(strcmp(m.name, registered_message.name) == 0)
        {
            // we need check for private-ness when private is supported
            // for now this is an error

            // at least check if the mechanism is the same
            DYNAMIX_ASSERT_MSG(false, "Attempting to register a message that has already been registered");

            // try to resque the situation in some way
            m.id = registered_message.id;

            return;
        }
    }

    if (free == INVALID_FEATURE_ID)
    {
        DYNAMIX_ASSERT_MSG(_num_registered_messages < DYNAMIX_MAX_MESSAGES,
            "you have to increase the maximum number of messages");

        m.id = _num_registered_messages;
        ++_num_registered_messages;
    }
    else
    {
        m.id = free;
    }

    _messages[m.id] = &m;
}

void domain::internal_unregister_feature(message_t& msg)
{
    DYNAMIX_ASSERT_MSG(msg.id < _num_registered_messages, "unregistering a message which isn't registered");
    auto* registered = _messages[msg.id];
    DYNAMIX_ASSERT_MSG(registered, "unregistering a message which isn't registered");
    DYNAMIX_ASSERT_MSG(registered == &msg, "unregistering a message with know id but unknown data");

    _messages[msg.id] = nullptr;

    // to be pedantic we should clear all type infos which have this message,
    // but this seems to be unnecessary
    // if a message has found its way into a type info, then there must be a mixin that uses it
    // if the message is local to a module, then that mixin must be local to the module too
    // since the message is being unregistered, because the module is unloaded
    // then surely the mixin should be unregistered too
    // then all types infos containing that mixin (and in turn this message)
    // will be dropped
}

void domain::internal_register_mixin_type(mixin_type_info& info)
{
    // as is the case with messages, multiple modules may turn out
    // trying to register the same mixin over again
    // due to module specific template instantiation
    // check if we already have this mixin registered

    mixin_id free = INVALID_MIXIN_ID;

    for (size_t i = 0; i < _num_registered_mixins; ++i)
    {
        mixin_type_info* registered = _mixin_type_infos[i];

        if (registered)
        {
            DYNAMIX_ASSERT(registered->is_valid());
            DYNAMIX_ASSERT_MSG(strcmp(info.name, registered->name) != 0, "registering the same mixin twice");
        }
        else
        {
            free = i;
        }
    }

    if (free == INVALID_MIXIN_ID)
    {
        // no free slot has been found dugin the iteration, so add a new one
        DYNAMIX_ASSERT_MSG(_num_registered_mixins < DYNAMIX_MAX_MIXINS,
            "you have to increase the maximum number of mixins");

        info.id = _num_registered_mixins;
        ++_num_registered_mixins;
    }
    else
    {
        info.id = free;
    }

    _mixin_type_infos[info.id] = &info;
}

void domain::unregister_mixin_type(const mixin_type_info& info)
{
    DYNAMIX_ASSERT_MSG(info.id < _num_registered_mixins, "unregistering a mixin which isn't registered");
    mixin_type_info* const registered = _mixin_type_infos[info.id];
    DYNAMIX_ASSERT_MSG(registered, "unregistering a mixin which isn't registered");
    DYNAMIX_ASSERT_MSG(registered == &info, "unregistering a mixin with known id but unknown data");

    _mixin_type_infos[info.id] = nullptr;

    // since this mixin is no longer valid
    // clean up all object type infos which reference it

#if DYNAMIX_THREAD_SAFE_MUTATIONS
    // TODO C++17: do this with a shared mutex instead
    std::lock_guard<std::mutex> lock(_object_type_infos_mutex);
#endif

    for (auto i = _object_type_infos.begin(); i != _object_type_infos.end(); )
    {
        if (i->first[info.id])
        {
            delete i->second;
            i = _object_type_infos.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

void domain::set_allocator(global_allocator* allocator)
{
    DYNAMIX_ASSERT(!_allocator || !_allocator->has_allocated());

    for(size_t i=0; i<_num_registered_mixins; ++i)
    {
        mixin_type_info* registered = _mixin_type_infos[i];

        if (!registered) continue;

        if(registered->allocator == _allocator)
        {
            // set the new domain allocator to all mixins that don't
            // have a specific one of their own
            registered->allocator = allocator;
        }
    }

    _allocator = allocator;
}

mixin_id domain::get_mixin_id_by_name(const char* mixin_name) const
{
    for(size_t i=0; i<_num_registered_mixins; ++i)
    {
        const mixin_type_info* registered = _mixin_type_infos[i];

        if (!registered) continue;

        if(strcmp(mixin_name, registered->name) == 0)
        {
            return registered->id;
        }
    }

    // no mixin of this name found
    return INVALID_MIXIN_ID;
}

} // namespace internal

mutation_rule_id add_new_mutation_rule(mutation_rule* rule)
{
    return internal::domain::safe_instance().add_mutation_rule(rule);
}

mutation_rule_id add_mutation_rule(mutation_rule* rule)
{
    return internal::domain::safe_instance().add_mutation_rule(rule);
}

mutation_rule_id add_mutation_rule(std::shared_ptr<mutation_rule> rule)
{
    return internal::domain::safe_instance().add_mutation_rule(rule);
}

std::shared_ptr<mutation_rule> remove_mutation_rule(mutation_rule_id id)
{
    return internal::domain::safe_instance().remove_mutation_rule(id);
}

// set allocator to all domains
void set_global_allocator(global_allocator* allocator)
{
    internal::domain::safe_instance().set_allocator(allocator);
}

} // namespace dynamix
