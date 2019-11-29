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
#include "object_type_info.hpp"
#include "feature.hpp"
#include "feature_parser.hpp"
#include "message.hpp"

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

    template <typename Mixin>
    void register_mixin_type(mixin_type_info& info)
    {
        DYNAMIX_ASSERT(info.id == INVALID_MIXIN_ID);

        info.size = sizeof(Mixin);
        info.alignment = std::alignment_of<Mixin>::value;
        info.constructor = &call_mixin_constructor<Mixin>;
        info.destructor = &call_mixin_destructor<Mixin>;
        info.copy_constructor = get_mixin_copy_constructor<Mixin>();
        info.copy_assignment = get_mixin_copy_assignment<Mixin>();
        info.move_constructor = get_mixin_move_constructor<Mixin>();
        info.move_assignment = get_mixin_move_assignment<Mixin>();
        info.allocator = _mixin_allocator();

#if DYNAMIX_USE_TYPEID
        info.name = get_mixin_name_from_typeid(typeid(Mixin).name());
#   if defined(__GNUC__)
        info.owns_name = true;
#   endif
#elif DYNAMIX_USE_STATIC_MEMBER_NAME
        // defining DYNAMIX_USE_STATIC_MEMBER_NAME means that you must provide
        // mixin names with a static const char* member function
        info.name = Mixin::dynamix_mixin_name();
#endif

        // see comments in feature_instance on why this manual registration is needed
        feature_registrator reg;
        _dynamix_parse_mixin_features(static_cast<Mixin*>(nullptr), reg);

        if (reg.mixin_name)
        {
#if DYNAMIX_USE_TYPEID && defined(__GNUC__)
            if (info.name && info.owns_name) free_mixin_name_from_typeid(info.name);
            info.owns_name = false;
#endif
            // override if available
            info.name = reg.mixin_name;
        }

        DYNAMIX_ASSERT_MSG(info.name, "Mixin name must be provided through a feature");

        internal_register_mixin_type(info);

        feature_parser<Mixin> parser;
        _dynamix_parse_mixin_features(static_cast<Mixin*>(nullptr), parser);
    }

    void unregister_mixin_type(const mixin_type_info& info);

    template <typename Feature>
    void register_feature(Feature& feature)
    {
        // see comments in feature_instance on why features may be registered multiple times
        if(feature.id != INVALID_FEATURE_ID)
        {
            return;
        }

        internal_register_feature(feature);
    }

    template <typename Feature>
    void unregister_feature(Feature& feature)
    {
        internal_unregister_feature(feature);
    }

    // creates a new type info if needed
    const object_type_info* get_object_type_info(const mixin_type_info_vector& mixins);

    const mixin_type_info& mixin_info(mixin_id id) const
    {
        DYNAMIX_ASSERT(id != INVALID_MIXIN_ID);
        DYNAMIX_ASSERT(id <= _num_registered_mixins);
        DYNAMIX_ASSERT(_mixin_type_infos[id]);

        return *_mixin_type_infos[id];
    }

    // sets the current domain allocator
    void set_allocator(domain_allocator* allocator);
    domain_allocator* allocator() const { return _allocator; }

    // get mixin id by name string
    mixin_id get_mixin_id_by_name(const char* mixin_name) const;

    // erases all type infos with zero objects
    void garbage_collect_type_infos();

_dynamix_internal:

    domain();
    ~domain();

    // non-copyable
    domain(const domain&) = delete;
    domain& operator=(const domain&) = delete;

    // sparse list of all mixin infos
    // some elements might be nullptr
    // such elements have been registered from a loadable module (plugin)
    // and then unregistered when it was unloaded
    mixin_type_info* _mixin_type_infos[DYNAMIX_MAX_MIXINS];
    size_t _num_registered_mixins; // max registered mixin

    void internal_register_mixin_type(mixin_type_info& info);

    // sparse list of all message infos
    // some elements might be nullptr
    // such elements have been registered from a loadable module (plugin)
    // and then unregistered when it was unloaded
    message_t* _messages[DYNAMIX_MAX_MESSAGES];
    size_t _num_registered_messages;

    typedef std::unordered_map<available_mixins_bitset, object_type_info*> object_type_info_map;

    object_type_info_map _object_type_infos;

    std::vector<std::shared_ptr<mutation_rule>> _mutation_rules;

#if DYNAMIX_THREAD_SAFE_MUTATIONS
    std::mutex _object_type_infos_mutex;
    std::mutex _mutation_rules_mutex;
#endif

    // feature registration functions for the supported kinds of features
    void internal_register_feature(message_t& m);
    void internal_unregister_feature(message_t& m);


    // allocators

    // we need this silly function in order to perform the cast from domain to mixin allocator
    // when allocators are included
    mixin_allocator* _mixin_allocator() const;
    domain_allocator* _allocator;

private:
    static const domain& _instance; // used for the fast version of the instance getter
};

} // namespace internal

// allocator functions

/// Sets an global allocator for all mixins and datas.
void DYNAMIX_API set_global_allocator(domain_allocator* allocator);

} // namespace dynamix
