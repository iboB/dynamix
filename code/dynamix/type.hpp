// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/api.h"
#include "../dnmx/basic_type.h"
#include "../dnmx/type_handle.h"

#include "size.hpp"
#include "mixin_index.hpp"
#include "mixin_id.hpp"
#include "feature_id.hpp"
#include "feature_payload.hpp"
#include "allocator.hpp"
#include "type_class.hpp"
#include "feature_info_fwd.hpp"
#include "mixin_info_fwd.hpp"
#include "globals.hpp"

#include <itlib/span.hpp>
#include <itlib/atomic.hpp>

#include <cstdint>
#include <string_view>

namespace dynamix {

class domain;

class DYNAMIX_API type : public dnmx_basic_type {
    friend class domain;
    type(domain& dom) noexcept : dnmx_basic_type({nullptr, 0}), dom(dom) {}
public:
    type(const type&) = delete;
    type& operator=(const type&) = delete;

    domain& dom;

    using ftable_payload = dnmx_ftable_payload;
    using ftable_entry = dnmx_ftable_entry;

    // compact array of mixins infos of this type, no null items
    itlib::span<const mixin_info* const> mixins;

    // size of mixin buffer for objects of this type
    byte_size_t object_buffer_size = 0;

    // alignment of object buffer: alignment of most aligned mixin, but at least sizeof_ptr
    byte_size_t object_buffer_alignment = 0;

    // offsets of mixins within the object buffer
    // a zero offset means an external mixin
    itlib::span<const uint32_t> mixin_offsets;

    // indices of mixins in m_mixins per mixin_id
    // if an index is invalid_mixin_index, it is not a part of this type
    itlib::span<const mixin_index_t> sparse_mixin_indices;

    // number of objects of this type
    // more precisely this is the number of active (living-allocated) object buffers
    size_t num_objects() const noexcept { return m_num_objects.load(); }

    mixin_index_t num_mixins() const noexcept {
        return mixin_index_t(mixins.size());
    }

    [[nodiscard]] FORCE_INLINE mixin_index_t index_of(mixin_id id) const noexcept {
        if (id.i >= sparse_mixin_indices.size()) return invalid_mixin_index;
        return sparse_mixin_indices[id.i];
    }
    [[nodiscard]] mixin_index_t index_of(std::string_view name) const noexcept;

    [[nodiscard]] bool has(const mixin_info& info) const noexcept;
    [[nodiscard]] FORCE_INLINE bool has(mixin_id id) const noexcept {
        return index_of(id) != invalid_mixin_index;
    }
    [[nodiscard]] bool has(std::string_view name) const noexcept {
        return index_of(name) != invalid_mixin_index;
    }

    [[nodiscard]] bool lacks(const mixin_info& info) const noexcept { return !has(info); }
    [[nodiscard]] bool lacks(mixin_id id) const noexcept { return !has(id); }
    [[nodiscard]] bool lacks(std::string_view name) const noexcept { return !has(name); }

    [[nodiscard]] bool implements_strong(const feature_info& info) const noexcept;
    [[nodiscard]] bool implements_strong(feature_id id) const noexcept;
    [[nodiscard]] bool implements_strong(std::string_view name) const noexcept;
    [[nodiscard]] bool implements(const feature_info& info) const noexcept;

    template <typename Mixin>
    [[nodiscard]] bool has() const noexcept { return has(g::get_mixin_info<Mixin>()); }
    template <typename Mixin>
    [[nodiscard]] bool lacks() const noexcept { return !has<Mixin>(); }
    template <typename Feature>
    [[nodiscard]] bool implements_strong() const noexcept { return implements_strong(g::get_feature_info_fast<Feature>()); }
    template <typename Feature>
    [[nodiscard]] bool implements() const noexcept { return implements(g::get_feature_info_fast<Feature>()); }

    // find the implemeter of feature after mixin
    // if:
    //  the mixin is a part of this type
    //  the mixin implements the feature
    //  has overridden another mixin which implements it
    //  -- returns the next overridden implementer
    // if any of the conditions is not true, returns nullptr
    const ftable_payload* find_next_implementer(const feature_info& feature, const mixin_info& mixin) const noexcept;

    // find the next bidder set for feature after mixin
    // if:
    //  the mixin is a part of this type
    //  the mixin implements the feature
    //  has overridden by a higher bid any other mixins which implements it
    //  -- returns the overridden implementers with the next bid
    // if any of the conditions is not true, returns an empty span
    itlib::span<const ftable_payload> find_next_bidder_set(const feature_info& feature, const mixin_info& mixin) const noexcept;

    // type class
    [[nodiscard]] bool is_of(const type_class& tc) const noexcept {
        return tc.matches(this);
    }
    // check by registered type class from domain
    // will throw domain_error if no such type class is registered
    [[nodiscard]] bool is_of(std::string_view name) const;
    template <typename TypeClass>
    [[nodiscard]] bool is_of() const noexcept {
        return is_of(TypeClass::m_dynamix_type_class);
    }

    // compare types
    // this will only return 0 if other is a reference to this type
    // otherwise it compares domains by address and if the domain is the same,
    // lexicographically compares mixins by canonical order
    int compare(const type& other) const noexcept;

    // as compare: only equal to itself
    bool operator==(const type& other) const noexcept { return this == &other; }
    bool operator!=(const type& other) const noexcept { return this != &other; }

    // traits

    // check if each mixin has a default init func
    bool default_constructible() const noexcept;

    // check if each mixin has a copy init func
    bool copy_constructible() const noexcept;

    // check if each mixin has a copy assign func
    bool copy_assignable() const noexcept;

    bool copyable() const noexcept { return copy_assignable() && copy_constructible(); }

    // check if each mixin has at least one of funcs equals or compare
    bool equality_comparable() const noexcept;

    // check if each mixin has an compare func
    bool comparable() const noexcept;

    // these are used by objects to manage their buffers
    // there don't seem to be reasons for library users to call them
    [[nodiscard]] byte_t* allocate_object_buffer(allocator& alloc) const;
    void deallocate_object_buffer(allocator& alloc, void* ptr) const noexcept;

    static const type* from_c_handle(dnmx_type_handle ht) noexcept { return static_cast<const type*>(ht); }
private:
    // size of own associated buffer
    // the domain allocates a single buffer for a type and all of its members
    // this is the buffer size (it's used when deallocating)
    byte_size_t m_buf_size = 0;

    mutable itlib::atomic_relaxed_counter<size_t> m_num_objects = {};
};

}
