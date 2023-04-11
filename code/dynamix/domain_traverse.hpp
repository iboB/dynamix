// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/api.h"
#include "feature_info_fwd.hpp"
#include "mixin_info_fwd.hpp"
#include "mutation_rule_info_fwd.hpp"
#include "type_class.hpp"

#include <itlib/span.hpp>
#include <functional>
#include <cstdint>

namespace dynamix {
class domain;
class type;

// this class is used to traverse the domain's registries
// it is used dump the domain's state in dbg_dmp
// it can also be used for ther debugging and testing purposes
// implemented in domain.cpp
class DYNAMIX_API domain_traverse {
    // TODO: make into fast pimpl buffer to avoid all allocs
    struct impl;
    impl* m_impl;
public:
    domain_traverse(const domain& d) noexcept;
    ~domain_traverse();

    domain_traverse(const domain_traverse&) = delete;
    domain_traverse& operator=(const domain_traverse&) = delete;

    void traverse_mixins(std::function<void(const mixin_info&)> f) const;
    void traverse_features(std::function<void(const feature_info&)> f) const;
    void traverse_mutation_rules(std::function<void(const mutation_rule_info&, uint32_t)> f) const;
    void traverse_type_classes(std::function<void(const type_class&)> f) const;
    void traverse_types(std::function<void(const type&)> f) const;
    void traverse_type_queries(std::function<void(itlib::span<const mixin_info* const>, const type&)> f) const;
};
}
