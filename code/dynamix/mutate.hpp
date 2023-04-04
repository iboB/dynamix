// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "mutate_ops.hpp"
#include "mutate_to.hpp"
#include "domain.hpp"

namespace dynamix {
// regular mutate with ops as arguments
template <typename... Ops>
void mutate(object& obj, Ops&&... ops) {
    type_mutation type_mut(obj.get_type());

    (ops.apply_to_type_mutation(type_mut), ...);

    auto& type = obj.get_domain().get_type(std::move(type_mut));

    mutate_to(obj, type, std::forward<Ops>(ops)...);
}

// v1 comaptible mutate which only works with mixins with default init functions
// not extensible
class gradual_mutation {
    type_mutation m_type_mut;
    object& m_obj;
    bool m_applied = false;
public:
    gradual_mutation(object& obj)
        : m_type_mut(obj.get_type())
        , m_obj(obj)
    {}
    ~gradual_mutation() noexcept(false) {
        if (std::uncaught_exceptions()) return;
        if (m_applied) return;
        apply();
    }

    gradual_mutation(const gradual_mutation&) = delete;
    gradual_mutation& operator=(const gradual_mutation&) = delete;

    using self_t = gradual_mutation;

    template <typename Mixin>
    self_t& add() {
        m_type_mut.add<Mixin>();
        return *this;
    }
    self_t& add(const mixin_info& info) {
        m_type_mut.add(info);
        return *this;
    }
    self_t& add(std::string_view name) {
        m_type_mut.add(name);
        return *this;
    }

    template <typename Mixin>
    self_t& add_if_lacking() {
        m_type_mut.add_if_lacking<Mixin>();
        return *this;
    }
    self_t& add_if_lacking(const mixin_info& info) {
        m_type_mut.add_if_lacking(info);
        return *this;
    }
    self_t& add_if_lacking(std::string_view name) {
        m_type_mut.add_if_lacking(name);
        return *this;
    }

    template <typename Mixin>
    self_t& remove() {
        m_type_mut.remove<Mixin>();
        return *this;
    }
    self_t& remove(const mixin_info& info) {
        m_type_mut.remove(info);
        return *this;
    }
    self_t& remove(std::string_view name) {
        m_type_mut.remove(name);
        return *this;
    }

    template <typename Mixin>
    self_t& to_back() {
        m_type_mut.to_back<Mixin>();
        return *this;
    }
    self_t& to_back(const mixin_info& info) {
        m_type_mut.to_back(info);
        return *this;
    }
    self_t& to_back(std::string_view name) {
        m_type_mut.to_back(name);
        return *this;
    }

    void apply() {
        auto& type = m_obj.get_domain().get_type(std::move(m_type_mut));
        m_obj.reset_type(type);
    }
};
inline gradual_mutation mutate(object& obj) {
    return gradual_mutation(obj);
}
}
