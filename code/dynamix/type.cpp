// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "type.hpp"
#include "mixin_info.hpp"
#include "feature_info.hpp"
#include "feature_for_mixin.hpp"
#include "domain.hpp"
#include "type_class.hpp"
#include "throw_exception.hpp"

#include <itlib/qalgorithm.hpp>

namespace dynamix {

mixin_index_t type::index_of(std::string_view name) const noexcept {
    for (mixin_index_t i = 0; i < mixins.size(); ++i) {
        if (mixins[i]->name == name) return i;
    }
    return invalid_mixin_index;
}

bool type::has(const mixin_info& info) const noexcept {
    return has(info.id);
}
bool type::implements_strong(const feature_info& info) const noexcept {
    return implements_strong(info.id);
}
bool type::implements_strong(feature_id id) const noexcept {
    return !!ftable_at(id);
}
bool type::implements_strong(std::string_view name) const noexcept {
    for (uint32_t i = 0; i < ftable_length; ++i) {
        auto& e = ftable[i];
        if (!e) continue;
        if (e.begin->data->info->name == name) return true;
    }
    return false;
}
bool type::implements(const feature_info& info) const noexcept {
    if (info.default_payload) return true;
    return implements_strong(info.id);
}

namespace {
const type::ftable_payload* find_implementer(const type::ftable_entry& fe, mixin_index_t mixin_index) noexcept {
    for (auto i = fe.begin; i != fe.end; ++i) {
        if (i->mixin_index == mixin_index) return i;
    }
    return nullptr;
}
}

const type::ftable_payload* type::find_next_implementer(const feature_info& feature, const mixin_info& mixin) const noexcept {
    auto fe = ftable_at(feature.id);
    if (!fe) return nullptr; // type does not implement feature

    auto mixin_index = index_of(mixin.id);
    auto f = find_implementer(fe, mixin_index);
    if (!f) return nullptr; // type does not implement feature for provided mixin
    ++f;
    if (f == fe.end) return nullptr; // provided mixin was the last - there is no next implementer
    return f;
}

itlib::span<const type::ftable_payload> type::find_next_bidder_set(const feature_info& feature, const mixin_info& mixin) const noexcept {
    auto fe = ftable_at(feature.id);
    if (!fe) return {}; // type does not implement feature

    auto mixin_index = index_of(mixin.id);
    auto f = find_implementer(fe, mixin_index);
    if (!f) return {}; // type does not implement feature for provided mixin
    auto fbid = f->data->bid;
    while (f != fe.end && f->data->bid == fbid) ++f; // find the end of the current bidder set
    if (f == fe.end) return {}; // provided mixin was of the last bidder set - there is no next one

    auto begin = f;
    auto end = begin + 1;
    while (end != fe.end && end->data->bid == f->data->bid) ++end; // find the end of the next bidder set
    return {begin, end};
}

bool type::is_of(std::string_view name) const {
    auto tc = dom.get_type_class(name);
    if (!tc) throw_exception::unknown_type_class(*this, name);
    return is_of(*tc);
}

int type::compare(const type& other) const noexcept {
    if (this == &other) return 0;
    if (&dom != &other.dom) return &dom < &other.dom ? -1 : 1;

    // C++20 std::lexicographical_compare_three_way
    auto m = mixins.begin();
    const auto mend = mixins.end();
    auto o = other.mixins.begin();
    const auto oend = other.mixins.end();
    while (true) {
        if (m == mend) {
            assert(o != oend); // sanity: how is this the same type?
            return -1;
        }

        if (o == oend) return 1;

        if (*m != *o) {
            canonical_mixin_order cmp;
            return cmp(*m, *o) ? -1 : 1;
        }

        ++m;
        ++o;
    }
}

bool type::default_constructible() const noexcept {
    return itlib::qall_of(mixins, [](const mixin_info* m) { return !!m->init; });
}

bool type::copy_constructible() const noexcept {
    return itlib::qall_of(mixins, [](const mixin_info* m) { return !!m->copy_init; });
}

bool type::copy_assignable() const noexcept {
    return itlib::qall_of(mixins, [](const mixin_info* m) { return !!m->copy_asgn; });
}

bool type::equality_comparable() const noexcept {
    return itlib::qall_of(mixins, [](const mixin_info* m) { return m->equals || m->compare; });
}

bool type::comparable() const noexcept {
    return itlib::qall_of(mixins, [](const mixin_info* m) { return !!m->compare; });
}

byte_t* type::allocate_object_buffer(allocator& alloc) const {
    auto ret = alloc.allocate_bytes(object_buffer_size, object_buffer_alignment);
    ++m_num_objects;
    return static_cast<byte_t*>(ret);
}

void type::deallocate_object_buffer(allocator& alloc, void* ptr) const noexcept {
    alloc.deallocate_bytes(ptr, object_buffer_size, object_buffer_alignment);
    --m_num_objects;
}

}
