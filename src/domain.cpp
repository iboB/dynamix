// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include "dynamix/domain.hpp"
#include "zero_memory.hpp"
#include "dynamix/object_type_info.hpp"
#include "dynamix/mutation_rule.hpp"
#include "dynamix/allocators.hpp"
#include "dynamix/internal/mixin_traits.hpp"
#include "dynamix/features.hpp"

#include <algorithm>

namespace dynamix
{

noop_feature_t* none;

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

domain::~domain() = default;

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

void domain::apply_mutation_rules(object_type_mutation& mutation, const mixin_collection& source_mixins)
{
#if DYNAMIX_THREAD_SAFE_MUTATIONS
    std::lock_guard<std::mutex> lock(_mutation_rules_mutex);
#endif

    for (auto& rule : _mutation_rules)
    {
        if (rule)
        {
            rule->apply_to(mutation, source_mixins);
        }
    }
}

const object_type_info* domain::get_object_type_info(mixin_collection mixins)
{
    // the mixin type infos need to be sorted
    // so as to guarantee that two object type infos of the same mixins
    // will have the exact same content
    I_DYNAMIX_ASSERT(std::is_sorted(mixins._compact_mixins.begin(), mixins._compact_mixins.end()));

#if DYNAMIX_THREAD_SAFE_MUTATIONS
    // TODO C++17: do this with a shared mutex instead
    std::lock_guard<std::mutex> lock(_object_type_infos_mutex);
#endif

    object_type_info_map::iterator it = _object_type_infos.find(mixins._mixins);

    if(it != _object_type_infos.end())
    {
        // get existing
        I_DYNAMIX_ASSERT(mixins._compact_mixins == it->second->_compact_mixins);
        return it->second.get();
    }
    else
    {
        // create object type info
        // use unique_ptr since fill_call_table might throw
        std::unique_ptr<object_type_info> new_type(new object_type_info);
        new_type->_mixins = mixins._mixins;

        uint32_t index = 0;
        for(auto info : mixins._compact_mixins)
        {
            I_DYNAMIX_ASSERT(info);
            new_type->_mixin_indices[info->id] = index + object_type_info::MIXIN_INDEX_OFFSET;
            ++index;
        }

        new_type->_compact_mixins = std::move(mixins._compact_mixins);

        new_type->fill_call_table();

        auto ret = new_type.get();
        _object_type_infos.emplace(make_pair(std::move(mixins._mixins), std::move(new_type)));
        return ret;
    }
}

void domain::register_feature(message_t& m)
{
    // since messages get registered by registering mixins
    // registration can happen multiple times
    // it the message is registered, we have nothing more to do
    if(m.id != INVALID_FEATURE_ID)
    {
        return;
    }

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

        I_DYNAMIX_ASSERT(registered_message.id != INVALID_FEATURE_ID); // how could this happen?

        if(strcmp(m.name, registered_message.name) == 0)
        {
            // we need check for private-ness when private is supported
            // for now this is an error

            // at least check if the mechanism is the same
            I_DYNAMIX_ASSERT_MSG(false, "Attempting to register a message that has already been registered");

            // try to resque the situation in some way
            m.id = registered_message.id;

            return;
        }
    }

    if (free == INVALID_FEATURE_ID)
    {
        I_DYNAMIX_ASSERT_MSG(_num_registered_messages < DYNAMIX_MAX_MESSAGES,
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

void domain::unregister_feature(message_t& msg)
{
    I_DYNAMIX_ASSERT_MSG(msg.id < _num_registered_messages, "unregistering a message which isn't registered");
    I_DYNAMIX_ASSERT_MSG(_messages[msg.id], "unregistering a message which isn't registered");
    I_DYNAMIX_ASSERT_MSG(_messages[msg.id] == &msg, "unregistering a message with know id but unknown data");

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

void domain::register_mixin_type(mixin_type_info& info)
{
    // mixin is already registered?
    I_DYNAMIX_ASSERT(info.id == INVALID_MIXIN_ID);

    // if this assert fails, there is no mixin name
    // you must either enable DYNAMIX_USE_TYPEID or DYNAMIX_USE_STATIC_MEMBER_NAME
    // or provide the name through a feature
    I_DYNAMIX_ASSERT_MSG(info.name, "Mixin name must be provided");

    mixin_id free = INVALID_MIXIN_ID;

    // TODO: optimize this check
    for (size_t i = 0; i < _num_registered_mixins; ++i)
    {
        mixin_type_info* registered = _mixin_type_infos[i];

        if (registered)
        {
            I_DYNAMIX_ASSERT(registered->is_valid());
            I_DYNAMIX_ASSERT_MSG(strcmp(info.name, registered->name) != 0, "registering the same mixin twice");
        }
        else
        {
            free = i;
        }
    }

    if (free == INVALID_MIXIN_ID)
    {
        // no free slot has been found dugin the iteration, so add a new one
        I_DYNAMIX_ASSERT_MSG(_num_registered_mixins < DYNAMIX_MAX_MIXINS,
            "you have to increase the maximum number of mixins");

        info.id = _num_registered_mixins;
        ++_num_registered_mixins;
    }
    else
    {
        info.id = free;
    }

    // also set allocator
    if (!info.allocator)
    {
        info.allocator = _allocator;
    }

    _mixin_type_infos[info.id] = &info;
}

void domain::unregister_mixin_type(const mixin_type_info& info)
{
    I_DYNAMIX_ASSERT_MSG(info.id < _num_registered_mixins, "unregistering a mixin which isn't registered");
    I_DYNAMIX_ASSERT_MSG(_mixin_type_infos[info.id], "unregistering a mixin which isn't registered");
    I_DYNAMIX_ASSERT_MSG(_mixin_type_infos[info.id] == &info, "unregistering a mixin with known id but unknown data");

#if DYNAMIX_USE_TYPEID && defined(__GNUC__)
    // if the name wasn't overriden by a feature, it has been obtained through
    // cxa_demangle and we must free it
    if(info.owns_name)
        free_mixin_name_from_typeid(info.name);
#endif

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
            i = _object_type_infos.erase(i);
        }
        else
        {
            ++i;
        }
    }
}

void domain::set_allocator(domain_allocator* allocator)
{
    I_DYNAMIX_ASSERT(!_allocator || !_allocator->has_allocated());

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

void domain::garbage_collect_type_infos()
{
    for (auto i = _object_type_infos.begin(); i != _object_type_infos.end(); )
    {
        if (i->second->num_objects == 0)
        {
            i = _object_type_infos.erase(i);
        }
        else
        {
            ++i;
        }
    }
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
void set_global_allocator(domain_allocator* allocator)
{
    internal::domain::safe_instance().set_allocator(allocator);
}

} // namespace dynamix
