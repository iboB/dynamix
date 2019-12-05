// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "config.hpp"
#include "mutation_rule_id.hpp"
#include "mixin_type_info.hpp"
#include "feature.hpp"
#include "message.hpp"
#include "mixin_collection.hpp" // for mixin_type_info_vector
#include "internal/assert.hpp"

#include <unordered_map>
#include <memory>
#include <type_traits> // alignment of

#if DYNAMIX_THREAD_SAFE_MUTATIONS
#include <mutex>
#endif

/**
 * \file
 * Domain related classes and functions.
 */

// The domain collection of mixins and messages
// It serves as a library instance of sorts

namespace dynamix
{

class mutation_rule;
class object_type_mutation;
class domain_allocator;

namespace internal
{

struct message_t;
class object_type_info;

class DYNAMIX_API domain
{
public:
    // contains static local variable which has thread-safe initialization
    // so this function is a bit slower, but it's safe to call globally
    static domain& safe_instance();

    // no static variables, not safe to call globally
    static const domain& instance();

    mutation_rule_id add_mutation_rule(std::shared_ptr<mutation_rule> rule);
    mutation_rule_id add_mutation_rule(mutation_rule* rule);
    std::shared_ptr<mutation_rule> remove_mutation_rule(mutation_rule_id id);
    void apply_mutation_rules(object_type_mutation& mutation);

    size_t num_registered_mixins() const { return _num_registered_mixins; }

    void register_mixin_type(mixin_type_info& info);
    void unregister_mixin_type(const mixin_type_info& info);

    // feature registration functions for the supported kinds of features
    void register_feature(message_t& m);
    void unregister_feature(message_t& m);

    // creates a new type info if needed
    const object_type_info* get_object_type_info(const mixin_type_info_vector& mixins);

    const mixin_type_info& mixin_info(mixin_id id) const
    {
        I_DYNAMIX_ASSERT(id != INVALID_MIXIN_ID);
        I_DYNAMIX_ASSERT(id <= _num_registered_mixins);
        I_DYNAMIX_ASSERT(_mixin_type_infos[id]);

        return *_mixin_type_infos[id];
    }

    const message_t& message_data(feature_id id) const
    {
        I_DYNAMIX_ASSERT(id <= _num_registered_messages);
        I_DYNAMIX_ASSERT(_messages[id]);
        return *_messages[id];
    }

    // sets the current domain allocator
    void set_allocator(domain_allocator* allocator);
    domain_allocator* allocator() const { return _allocator; }

    // get mixin id by name string
    mixin_id get_mixin_id_by_name(const char* mixin_name) const;

    // erases all type infos with zero objects
    void garbage_collect_type_infos();

private:
    domain();
    ~domain();

    friend class object_type_info;
    friend class object_mutator;

    // non-copyable
    domain(const domain&) = delete;
    domain& operator=(const domain&) = delete;

    // sparse list of all mixin infos
    // some elements might be nullptr
    // such elements have been registered from a loadable module (plugin)
    // and then unregistered when it was unloaded
    mixin_type_info* _mixin_type_infos[DYNAMIX_MAX_MIXINS];
    size_t _num_registered_mixins; // max registered mixin

    // sparse list of all message infos
    // some elements might be nullptr
    // such elements have been registered from a loadable module (plugin)
    // and then unregistered when it was unloaded
    message_t* _messages[DYNAMIX_MAX_MESSAGES];
    size_t _num_registered_messages;

    typedef std::unordered_map<available_mixins_bitset, std::unique_ptr<object_type_info>> object_type_info_map;
    object_type_info_map _object_type_infos;

    // mutation rules for this domain
    std::vector<std::shared_ptr<mutation_rule>> _mutation_rules;

#if DYNAMIX_THREAD_SAFE_MUTATIONS
    std::mutex _object_type_infos_mutex;
    std::mutex _mutation_rules_mutex;
#endif

    // allocators
    domain_allocator* _allocator;

    static const domain& _instance; // used for the fast version of the instance getter
};

} // namespace internal

// allocator functions

/// Sets an global allocator for all mixins and datas.
void DYNAMIX_API set_global_allocator(domain_allocator* allocator);

} // namespace dynamix
