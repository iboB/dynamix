// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/api.h"
#include "../dnmx/domain_handle.h"
#include "../dnmx/basic_domain.h"

#include "domain_settings.hpp"
#include "feature_info_fwd.hpp"

#include "feature_id.hpp"
#include "mixin_id.hpp"
#include "mixin_info_fwd.hpp"
#include "mutation_rule_info_fwd.hpp"

#include "allocator.hpp"

#include <itlib/span.hpp>

#include <string_view>

namespace dynamix {

class type;
class type_mutation;

// a domain is used to register mixins and features
// it manages object types
// an object is always "of a domain"
// mixins and features are not usable across domains
class DYNAMIX_API domain : private dnmx_basic_domain {
public:
    domain(std::string_view name = {}, domain_settings settings = {}, uintptr_t user_data = 0, void* context = nullptr, allocator alloc = {});
    ~domain();
    domain(const domain&) = delete;
    domain& operator=(const domain&) = delete;
    domain(domain&&) = delete;
    domain& operator=(domain&&) = delete;

    // this value is not used by the library but can be helpful in debugging multiple domains
    [[nodiscard]] std::string_view name() const noexcept { return m_name.to_std(); }

    [[nodiscard]] const domain_settings& settings() const noexcept { return m_settings; }

    // registrators
    // all registered infos for a domain are registered by address
    // the user is responsible for preserving the lifetime of the infos
    // with default settings get_type will register all unregistered mixins and features
    void register_feature(feature_info& info); // explicitly registering features is optional
    void register_mixin(mixin_info& info); // also register mixin's features

    // unregistrators
    // optionally unregister infos (they must have been registered sucessfully before that)
    // unregistrators also remove all object types which use such an info
    // the user is responsible to make sure that there are no living objects of such types
    // if objects do remain, using them in any way (even to destroy them) is ub (likely crash)
    void unregister_mixin(mixin_info& info);
    void unregister_feature(feature_info& info);

    // get registered infos
    // return nullptr if nothing matches the arg
    // these functions are not const, as they are not safe to use where a const domain
    // would be available for the risk of recursive mutex locks
    const mixin_info* get_mixin_info(mixin_id id) noexcept;
    const mixin_info* get_mixin_info(std::string_view name) noexcept;
    const feature_info* get_feature_info(feature_id id) noexcept;
    const feature_info* get_feature_info(std::string_view name) noexcept;

    // mutation rules
    // adding and removing the same mutation rule is safe (it is managed by an internal ref count)
    // adding the same rule to multiple domains is also safe
    // removing a rule which has not been added is safe (noop)
    void add_mutation_rule(const mutation_rule_info& info);
    void remove_mutation_rule(const mutation_rule_info& info) noexcept;

    // get a type from a mutation
    // employs mutation rules as needed
    // will return an existing type or create a new one if such doesn't exist
    // the mixins must be previously registered with the domain
    // the mixins in the list must be unique
    // the returned value will be invalidated if a pariticipating mixin gets unregistered
    const type& get_type(type_mutation&& mutation);

    // shorthand for getting a type by a given list
    // this will internally create a mutation from the empty type and run mutation rules
    // then return the requested (potentially new) type
    const type& get_type(itlib::span<const mixin_info* const> query);

    // performs garbage collection removing object types with zero objects
    void garbage_collect_types() noexcept;

    // get the domain's empty type
    // this type implements default implementations and has no mixins
    // disgregards mutation rules, so
    // this is not necessaritly equivalent to get_type({}) as mutation rules
    // may exist which add mixins to empty queries (such rules are evil, but que sera sera)
    const type& get_empty_type() const noexcept;

    // get number of existing types
    [[nodiscard]] size_t num_types() const noexcept;

    allocator get_allocator() const noexcept;

    using dnmx_basic_domain::user_data;
    using dnmx_basic_domain::context;

    dnmx_domain_handle to_c_hanlde() const noexcept { return this; }
    static domain* from_c_handle(dnmx_domain_handle hd) noexcept { return const_cast<domain*>(static_cast<const domain*>(hd)); }

    // only used in tests
    size_t num_type_queries() const noexcept;
    size_t num_mutation_rules() const noexcept;
private:
    // TODO: make into fast pimpl buffer to avoid all allocs
    class impl;
    impl* m_impl;
};

}
