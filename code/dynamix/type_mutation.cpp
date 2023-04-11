// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "type_mutation.hpp"

#include "type.hpp"
#include "domain.hpp"
#include "mixin_info.hpp"
#include "feature_info.hpp"
#include "feature_for_mixin.hpp"
#include "throw_exception.hpp"

#include <itlib/qalgorithm.hpp>
#include <itlib/stride_span.hpp>

#include <cassert>

namespace dynamix {

type_mutation::type_mutation(domain& d, const allocator& alloc) noexcept
    : dom(d)
    , mixins(alloc)
{}

type_mutation::type_mutation(const type& base, const allocator& alloc) noexcept
    : type_mutation(base.dom, alloc)
{
    mixins.assign(base.mixins.begin(), base.mixins.end());
}

#define by_name [&](const auto* info) { return info->name == name; }

static void do_to_back(compat::pmr::vector<const mixin_info*>::iterator i, compat::pmr::vector<const mixin_info*>& vec) {
    auto* val = *i;
    vec.erase(i);
    vec.push_back(val);
}

void type_mutation::to_back(const mixin_info& info) {
    auto f = itlib::qfind(mixins, &info);
    if (f == mixins.end()) {
        throw_exception::type_mut_error(*this, "to_back on missing mixin", info);
    }
    do_to_back(f, mixins);
}

const mixin_info& type_mutation::to_back(std::string_view name) {
    auto f = itlib::qfind_if(mixins, by_name);
    if (f == mixins.end()) {
        throw_exception::type_mut_error(*this, "to_back", name);
    }
    do_to_back(f, mixins);
    return *mixins.back();
}

bool type_mutation::remove(const mixin_info& info) noexcept {
    return itlib::erase_first(mixins, &info);
}
const mixin_info* type_mutation::remove(std::string_view name) noexcept {
    auto f = itlib::qfind_if(mixins, by_name);
    if (f == mixins.end()) return nullptr;
    auto ret = *f;
    mixins.erase(f);
    return ret;
}
bool type_mutation::has(const mixin_info& info) const noexcept {
    return !!itlib::pfind(mixins, &info);
}
const mixin_info* type_mutation::has(std::string_view name) const noexcept {
    auto f = itlib::pfind_if(mixins, by_name);
    if (f) return *f;
    return nullptr;
}

#define fspan(features) itlib::make_stride_span_member_view(features.data(), features.size(), &feature_for_mixin::info)

bool type_mutation::implements_strong(const feature_info& info) const noexcept {
    for (auto m : mixins) {
        auto features = m->features_span();
        auto span = fspan(features);
        if (itlib::pfind(span, &info)) return true;
    }
    return false;
}
const feature_info* type_mutation::implements_strong(std::string_view name) const noexcept {
    for (auto m : mixins) {
        auto features = m->features_span();
        auto span = fspan(features);
        auto f = itlib::pfind_if(span, by_name);
        if (f) return *f;
    }
    return nullptr;
}
bool type_mutation::implements(const feature_info& info) const noexcept {
    if (info.default_payload) return true;
    return implements_strong(info);
}

const mixin_info* type_mutation::safe_add(std::string_view name) {
    auto info = dom.get_mixin_info(name);
    if (!info) return nullptr;
    add(*info);
    return info;
}

const mixin_info& type_mutation::add(std::string_view name) {
    auto info = safe_add(name);
    if (!info) throw_exception::type_mut_error(*this, "add", name);
    return *info;
}

const mixin_info* type_mutation::safe_add_if_lacking(std::string_view name) {
    auto f = itlib::pfind_if(mixins, by_name);
    if (f) return nullptr;
    return safe_add(name);
}

const mixin_info* type_mutation::add_if_lacking(std::string_view name) {
    auto f = itlib::pfind_if(mixins, by_name);
    if (f) return nullptr;
    return &add(name);
}

void type_mutation::dedup() noexcept {
    // first mark as null
    for (auto i = mixins.begin(); i != mixins.end(); ++i) {
        auto& info = *i;
        assert(info);

        for (auto j = i + 1; j != mixins.end(); ++j) {
            if (info == *j) {
                // duplicate
                // remove the earlier
                info = nullptr;
            }
        }
    }

    // then erase
    itlib::erase_all(mixins, nullptr);
}

}
