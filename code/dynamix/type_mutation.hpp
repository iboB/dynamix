// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/api.h"
#include "../dnmx/type_mutation_handle.h"
#include "allocator.hpp"
#include "feature_info_fwd.hpp"
#include "mixin_info_fwd.hpp"
#include "globals.hpp"

#include "compat/pmr/vector.hpp"

#include <string_view>

namespace dynamix {

class domain;
class type;

// a class which represents a type mutation in progress
class DYNAMIX_API type_mutation {
public:
    domain& dom;
    compat::pmr::vector<const mixin_info*> mixins; // mixins of the not yet materialized type

    // start a mutation from the empty type
    // optionally provide an allocator for the type template
    explicit type_mutation(domain& d, const allocator& alloc = {}) noexcept;

    // start a mutation with a specific base
    // optionally provide an allocator for the type template
    explicit type_mutation(const type& base, const allocator& alloc = {}) noexcept;

    // add mixin
    void add(const mixin_info& info) { mixins.push_back(&info); }
    const mixin_info& add(std::string_view name); // can throw bad_mutation
    const mixin_info* safe_add(std::string_view name); // no bad_mutation if name is not a registered mixin
    template <typename Mixin>
    const mixin_info& add() {
        auto& info = g::get_mixin_info<Mixin>();
        add(info);
        return info;
    }

    // add if not already there
    bool add_if_lacking(const mixin_info& info) {
        if (has(info)) return false;
        add(info);
        return true;
    }
    const mixin_info* add_if_lacking(std::string_view name); // can throw bad_mutation
    const mixin_info* safe_add_if_lacking(std::string_view name); // no bad_mutation if name is not a registered mixin
    template <typename Mixin>
    bool add_if_lacking() {
        auto& info = g::get_mixin_info<Mixin>();
        return add_if_lacking(info);
    }

    // move mixin to back
    void to_back(const mixin_info& info);
    const mixin_info& to_back(std::string_view name);
    template <typename Mixin>
    void to_back() { to_back(g::get_mixin_info<Mixin>()); }

    // return false nothing was removed
    bool remove(const mixin_info& info) noexcept;
    const mixin_info* remove(std::string_view name) noexcept;
    template <typename Mixin>
    bool remove() noexcept { return remove(g::get_mixin_info<Mixin>()); }

    // deduplicate mixins
    // will leave the latter entry if a duplicate exists
    void dedup() noexcept;

    // queries for the not yet materialized type

    // these are linear searches so they may be slow
    // we don't have the indexes of a complete type here
    [[nodiscard]] bool has(const mixin_info& info) const noexcept;
    [[nodiscard]] const mixin_info* has(std::string_view name) const noexcept;
    [[nodiscard]] bool lacks(const mixin_info& info) const noexcept { return !has(info); }
    [[nodiscard]] bool lacks(std::string_view name) const noexcept { return !has(name); }
    [[nodiscard]] bool implements_strong(const feature_info& info) const noexcept;
    [[nodiscard]] const feature_info* implements_strong(std::string_view name) const noexcept;
    [[nodiscard]] bool implements(const feature_info& info) const noexcept;

    template <typename Mixin>
    [[nodiscard]] bool has() const noexcept { return has(g::get_mixin_info<Mixin>()); }
    template <typename Mixin>
    [[nodiscard]] bool lacks() const noexcept { return !has<Mixin>(); }
    template <typename Feature>
    [[nodiscard]] bool implements_strong() const noexcept { return implements_strong(g::get_feature_info_fast<Feature>()); }
    template <typename Feature>
    [[nodiscard]] bool implements() const noexcept { return implements(g::get_feature_info_fast<Feature>()); }


    dnmx_type_mutation_handle to_c_hanlde() noexcept { return reinterpret_cast<dnmx_type_mutation_handle>(this); }
    static type_mutation* from_c_handle(dnmx_type_mutation_handle ha) noexcept { return reinterpret_cast<type_mutation*>(ha); }
};

// create a type mutation from list of mixins
// there must be at least one (from which the relevant domain is retrieved)
// template <typename FirstMixin, typename... Rest>
// type_mutation make_type_mutation(const allocator& alloc = {}) {
    // compat::pmr::vector<const mixin_info*> mixins(alloc);
    // mixins = {&g::get_mixin_info<FirstMixin>(), &g::get_mixin_info<Rest>()...};
    // type_mutation ret(domain_from_info(*mixins.front()), alloc);
    // ret.mod_new_type().mixins = std::move(mixins);
    // return ret;
// }

}
