// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "type_mutation.hpp"

#include "type.hpp"
#include "domain.hpp"
#include "mixin_info.hpp"
#include "feature_info.hpp"
#include "feature_for_mixin.hpp"
#include "exception.hpp"

#include <itlib/qalgorithm.hpp>
#include <itlib/stride_span.hpp>

#include <cassert>

namespace dynamix {

type_mutation::type_mutation(domain& d, const allocator& alloc) noexcept
    : type_mutation(d.get_empty_type(), alloc)
{}

type_mutation::type_mutation(const type& base, const allocator& alloc) noexcept
    : m_base(base)
    , m_new_type(alloc)
{
    reset();
}

domain& type_mutation::get_domain() const noexcept { return m_base.dom; }

void type_mutation::reset() {
    m_new_type.mixins.assign(m_base.mixins.begin(), m_base.mixins.end());
}

#define by_name [&](const auto* info) { return info->name == name; }

static void do_to_back(compat::pmr::vector<const mixin_info*>::iterator i, compat::pmr::vector<const mixin_info*>& vec) {
    if (i == vec.end()) throw mutation_error("missing mixin");
    auto* val = *i;
    vec.erase(i);
    vec.push_back(val);
}

void type_mutation::type_template::to_back(const mixin_info& info) {
    do_to_back(itlib::qfind(mixins, &info), mixins);
}

const mixin_info& type_mutation::type_template::to_back(std::string_view name) {
    auto f = itlib::qfind_if(mixins, by_name);
    do_to_back(f, mixins);
    return *mixins.back();
}

bool type_mutation::type_template::remove(const mixin_info& info) noexcept {
    return itlib::erase_first(mixins, &info);
}
const mixin_info* type_mutation::type_template::remove(std::string_view name) noexcept {
    auto f = itlib::qfind_if(mixins, by_name);
    if (f == mixins.end()) return nullptr;
    auto ret = *f;
    mixins.erase(f);
    return ret;
}
bool type_mutation::type_template::has(const mixin_info& info) const noexcept {
    return !!itlib::pfind(mixins, &info);
}
const mixin_info* type_mutation::type_template::has(std::string_view name) const noexcept {
    auto f = itlib::pfind_if(mixins, by_name);
    if (f) return *f;
    return nullptr;
}

#define fspan(features) itlib::make_stride_span_member_view(features.data(), features.size(), &feature_for_mixin::info)

bool type_mutation::type_template::implements_strong(const feature_info& info) const noexcept {
    for (auto m : mixins) {
        auto features = m->features_span();
        auto span = fspan(features);
        if (itlib::pfind(span, &info)) return true;
    }
    return false;
}
const feature_info* type_mutation::type_template::implements_strong(std::string_view name) const noexcept {
    for (auto m : mixins) {
        auto features = m->features_span();
        auto span = fspan(features);
        auto f = itlib::pfind_if(span, by_name);
        if (f) return *f;
    }
    return nullptr;
}
bool type_mutation::type_template::implements(const feature_info& info) const noexcept {
    if (info.default_payload) return true;
    return implements_strong(info);
}

const mixin_info* type_mutation::safe_add(std::string_view name) {
    auto info = get_domain().get_mixin_info(name);
    if (!info) return nullptr;
    add(*info);
    return info;
}

const mixin_info& type_mutation::add(std::string_view name) {
    auto info = safe_add(name);
    if (!info) throw mutation_error("missing mixin name");
    return *info;
}

const mixin_info* type_mutation::safe_add_if_lacking(std::string_view name) {
    auto f = itlib::pfind_if(m_new_type.mixins, by_name);
    if (f) return nullptr;
    return safe_add(name);
}

const mixin_info* type_mutation::add_if_lacking(std::string_view name) {
    auto f = itlib::pfind_if(m_new_type.mixins, by_name);
    if (f) return nullptr;
    return &add(name);
}

bool type_mutation::noop() const noexcept {
    return std::equal(m_base.mixins.begin(), m_base.mixins.end(), m_new_type.mixins.begin(), m_new_type.mixins.end());
}

bool type_mutation::adding_mixins() const noexcept {
    for (auto new_info : m_new_type.mixins) {
        if (!m_base.has(new_info->id)) return true;
    }
    return false;
}

bool type_mutation::removing_mixins() const noexcept {
    for (auto new_info : m_base.mixins) {
        if (!m_new_type.has(*new_info)) return true;
    }
    return false;
}

bool type_mutation::adding(const mixin_info& info) const noexcept {
    if (m_base.has(info)) return false; // already in base
    return m_new_type.has(info);
}
const mixin_info* type_mutation::adding(std::string_view name) const noexcept {
    if (m_base.has(name)) return nullptr; // already in base
    return m_new_type.has(name);
}

bool type_mutation::removing(const mixin_info& info) const noexcept {
    if (!m_base.has(info)) return false; // not in base to remove
    return !m_new_type.has(info);
}
const mixin_info* type_mutation::removing(std::string_view name) const noexcept {
    auto bi = m_base.index_of(name);
    if (bi == invalid_mixin_index) return nullptr; // not in base to remove
    auto info = m_base.mixins[bi];
    if (m_new_type.has(*info)) return nullptr; // not removing
    return info;
}

void type_mutation::type_template::dedup() noexcept {
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
