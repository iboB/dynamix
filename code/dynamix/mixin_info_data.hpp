// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "common_mixin_info.hpp"
#include "common_feature_info.hpp"
#include "mixin_info_util.hpp"
#include "feature_for_mixin.hpp"
#include "allocator.hpp"
#include "globals.hpp"
#include "mutation_rule_info.hpp"
#include "mixin_allocator.hpp"
#include "any.hpp"
#include "mixin_info_data_literals.hpp"

#include <splat/warnings.h>

#include "compat/pmr/string.hpp"
#include "compat/pmr/vector.hpp"
#include "compat/pmr/deque.hpp"

namespace dynamix::util {

// a type which contains mixin_info with buffers for associated payloads
struct DYNAMIX_API mixin_info_data {
    explicit mixin_info_data(const allocator& alloc = {}) noexcept
        : stored_name(alloc)
        , feature_payloads(alloc)
        , feature_payload_storage(alloc)
        , mutation_rule_infos(alloc)
        , mutation_rule_info_storage(alloc)
    {}

    allocator get_allocator() const noexcept { return stored_name.get_allocator(); }

    common_mixin_info info;

    compat::pmr::string stored_name;

    t_any<mixin_allocator> stored_alloc;

    compat::pmr::vector<feature_for_mixin> feature_payloads;
    compat::pmr::deque<any> feature_payload_storage;

    compat::pmr::deque<const mutation_rule_info*> mutation_rule_infos;
    compat::pmr::deque<mutation_rule_info> mutation_rule_info_storage;

    // use these to register infos build from this data
    // these function will automatically manage the associated mutation rules
    void register_in(domain& dom);
    void unregister_from(domain& dom) noexcept;
};

// Now, an EDSL which builds mixin_info_data:

template <typename Mixin = void>
class mixin_info_data_builder {
    mixin_info_data& m_data;
public:
    mixin_info_data_builder(mixin_info_data& data, dnmx_sv name) noexcept
        : m_data(data)
    {
        m_data.info.name = name;
        common_init_mixin_info<Mixin>(m_data.info);
    }
    mixin_info_data_builder(mixin_info_data& d, std::string_view name) noexcept
        : mixin_info_data_builder(d, dnmx_sv::from_std(name))
    {}

    // adopting ctor: doesn't try to initialize data
    mixin_info_data_builder(std::nullptr_t, mixin_info_data& data) noexcept
        : m_data(data)
    {}

    allocator get_allocator() const noexcept { return m_data.get_allocator(); }

    mixin_info_data& get_data() const noexcept { return m_data; }

    using self = mixin_info_data_builder;

    // override name from view
    self& name(std::string_view name) noexcept {
        m_data.info.name = dnmx_sv::from_std(name);
        return *this;
    }

    // override and store store name
    self& store_name(std::string_view sname) {
        m_data.stored_name = sname;
        return name(m_data.stored_name);
    }

    // set allocator
    self& uses_allocator(t_any<mixin_allocator> alloc) {
        m_data.stored_alloc = alloc;
        m_data.info.allocator = m_data.stored_alloc->to_c_hanlde();
        return *this;
    }

    self& uses_allocator(mixin_allocator& alloc) {
        return uses_allocator(fwd_any(alloc));
    }

    template <typename Alloc>
    self& uses_allocator() {
        return uses_allocator(make_any<Alloc>(m_data.get_allocator()));
    }

    // user data
    self& user_data(uintptr_t data) {
        m_data.info.user_data = data;
        return *this;
    }

    // dependency status
    self& dependency(bool b = true) {
        m_data.info.dependency = b;
        return *this;
    }

    // add features
    self& implements_with_payload(const feature_info& info, any payload, builder_perks perks_a = {}, builder_perks perks_b = {}) {
        int32_t bid = perks_a.bid.value_or(perks_b.bid.value_or(0));
        int32_t priority = perks_a.priority.value_or(perks_b.priority.value_or(0));
        auto* stored = m_data.feature_payload_storage.emplace_back(std::move(payload)).get();
        m_data.feature_payloads.push_back({&info, stored, bid, priority});
        m_data.info.set_features(m_data.feature_payloads);
        return *this;
    }

    template <typename Payload>
    self& implements_by(const feature_info& info, Payload& pl, builder_perks perks_a = {}, builder_perks perks_b = {}) {
        return implements_with_payload(info, fwd_any(pl), perks_a, perks_b);
    }

    template <typename Payload>
    self& implements_with(const feature_info& info, Payload&& pl, builder_perks perks_a = {}, builder_perks perks_b = {}) {
        return implements_with_payload(info, make_any(get_allocator(), std::forward<Payload>(pl)), perks_a, perks_b);
    }

    template <typename Feature, typename Payload>
    self& implements_by(Payload& tpl, builder_perks perks_a = {}, builder_perks perks_b = {}) {
        using traits = typename Feature::traits;
        any pl = traits::make_payload_by(get_allocator(), impl::make_nullptr<Mixin>(), tpl);
        const feature_info& finfo = g::get_feature_info_safe<Feature>();
        return implements_with_payload(finfo, pl, perks_a, perks_b);
    }

    template <typename Feature, typename Payload>
    self& implements_by(const Payload& tpl, builder_perks perks_a = {}, builder_perks perks_b = {}) {
        using traits = typename Feature::traits;
        any pl = traits::make_payload_by(get_allocator(), impl::make_nullptr<Mixin>(), tpl);
        const feature_info& finfo = g::get_feature_info_safe<Feature>();
        return implements_with_payload(finfo, pl, perks_a, perks_b);
    }

    template <typename Feature, typename Payload>
    self& implements_with(Payload&& tpl, builder_perks perks_a = {}, builder_perks perks_b = {}) {
        using traits = typename Feature::traits;
        any pl = traits::make_payload_with(get_allocator(), impl::make_nullptr<Mixin>(), std::forward<Payload>(tpl));
        const feature_info& finfo = g::get_feature_info_safe<Feature>();
        return implements_with_payload(finfo, pl, perks_a, perks_b);
    }

    template <typename Feature>
    self& implements(builder_perks perks_a = {}, builder_perks perks_b = {}) {
        using traits = typename Feature::traits;
        any pl = traits::make_payload_for(get_allocator(), impl::make_nullptr<Mixin>());
        const feature_info& finfo = g::get_feature_info_safe<Feature>();
        return implements_with_payload(finfo, pl, perks_a, perks_b);
    }

    // add rules
    // WARNING: adds_mutation_rule implicitly makes the mixin a dependency
    // if you don't want this, you must add .dependency(false) after all adds_mutation_rule calls
    self& adds_mutation_rule(const mutation_rule_info& rule) {
        m_data.mutation_rule_infos.push_back(&rule);
        m_data.info.dependency = true;
        return *this;
    }

    self& adds_mutation_rule(error_return_t(apply)(dnmx_type_mutation_handle, uintptr_t), uintptr_t user_data = 0, builder_perks perks = {}) {
        int32_t priority = perks.priority.value_or(0);
        m_data.mutation_rule_info_storage.push_back({{}, apply, user_data, priority});
        return adds_mutation_rule(m_data.mutation_rule_info_storage.back());
    }

    // on mutation uses the common rule syntax to add template rules for this mixin
    // it does not imply the dependency status but uses the value from the common rule
    template <typename CommonRule>
    self& on_mutation(CommonRule&&, builder_perks perks = {}, uintptr_t user_data = 0) {
        auto r = CommonRule::template get_rule_for<Mixin>();
        if (r.makes_dependency) {
            m_data.info.dependency = true;
        }
        r.info.user_data = user_data;
        r.info.order_priority = perks.priority.value_or(0);
        m_data.mutation_rule_info_storage.push_back(r.info);
        m_data.mutation_rule_infos.push_back(&m_data.mutation_rule_info_storage.back());
        return *this;
    }

    // noop: useful for extending mixin macro with custom features and feature sets
    template <typename T>
    self& noop(T&&) { return *this; }
};

}
