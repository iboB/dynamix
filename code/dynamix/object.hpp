// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "../dnmx/api.h"
#include "../dnmx/basic_object.h"
#include "../dnmx/object_handle.h"

#include "mixin_id.hpp"
#include "mixin_index.hpp"
#include "allocator.hpp"
#include "object_mixin_data.hpp"
#include "mixin_info_fwd.hpp"
#include "type_class.hpp"
#include "globals.hpp"

#include <splat/inline.h>

#include <memory>
#include <string_view>

namespace dynamix {

class domain;
class type;
class object_mutation;
class mutation;

class DYNAMIX_API object : private dnmx_basic_object {
public:
    // constructs an empty object with no mixins
    // it however may still end-up implementing some features if there are ones with
    // default implementations registered with the domain
    explicit object(domain& dom, const allocator& alloc = {}) noexcept;

    // constructs an object from a type using the default constructors
    // may throw if the type is not default constructible or
    // the constructors from the type throw
    explicit object(const type& type, const allocator& alloc = {}, bool create_sealed = false);

    // constructs an object from a template
    // may throw if the constructors from the template throw
    // explicit object(const object_initializer& tpl, const allocator& alloc = {});

    ~object();

    // noexcept move - always safe
    // will also copy the allocator of source
    // source is empty afterwards
    object(object&& o) noexcept;

    // if allocators match, this does simple pointer copies and doesn't throw
    // if they don't, the op is not supported (yet?) and will throw bad_move
    object& operator=(object&& o);

    //////////////////////////
    // copying

    // implicit copying is disabled by default, though if you require it, you may inherit
    // from object and provide the copy functions
    // here's how it may look:
    // class copyable_obj : public dynamix::object {
    // public:
    //   using dynamix::object::object;
    //   copyable_obj(const copyable_obj& other) : dynamix::object(other.copy) {}
    //   copyable_obj& operator=(const copyable_obj& other) { copy_from(other); return *this; }
    //   // DON'T FORGET THESE (the copy ops from above hide them)
    //   copyable_obj(copyable_obj&& o) noexcept = default;
    //   copyable_obj& operator=(copyable_obj&& o) = default;
    // }
    object(const object& o) = delete;
    object& operator=(const object& o) = delete;

    // create a copy
    // note that this will slice the type if you use it witha a class inherited from object
    // may throw `bad_copy` if copy is not possible
    // optionally provide an allocator for the copy, otherwise the default is used
    [[nodiscard]] object copy(const allocator& alloc = {}) const;

    // explicit copy assignment from existing object
    // * changes the type of the target to the source type
    // * calls assignment operators for mixins that exist in both objects
    // * copy-constructs new mixins
    // * destroys mixins that don't exist in source.
    // * may throw `bad_copy` if copy is not possible
    // * has basic exception safety guarantees: if a copy of any member fails,
    //   the object will be valid but unpredictable (potentially "half-copied")
    //   the type won't be changed, but some mixins may be copy-assigned from the source
    //
    // to achieve a strong copy guarantee, you will have to ditch assignment
    // and copy-construct a helper to then move into this, like so:
    //    auto copy_helper = source.copy(); // if this throws, this_obj is not touched
    //    this_obj = std::move(copy_helper);
    void copy_from(const object& o);

    // copy-assign matching mixins from o
    // will not touch own type
    // will throw bad_copy if the matchin mixins have no copy-assignment
    // if any of the underlying copy ops throws, the object will end-up
    // in a semi-copied state
    void copy_matching_from(const object& o);

    // move-assigns matching mixins from o
    // will not touch own type
    // optionally fall-back to move-construction via arg if move assignment isn't available for a mixin
    // will throw bad_move if move is not possible
    void move_matching_from(object& o, bool fallback_to_init = false);

    //////////////////////////
    // general queries

    domain& get_domain() const noexcept;

    const type& get_type() const noexcept;

    bool is_of(const type_class& tc) const noexcept {
        return tc.matches(m_type);
    }
    bool is_of(std::string_view name) const; // will throw if type class is not registered
    template <typename TypeClass>
    bool is_of() const noexcept {
        return TypeClass::m_dynamix_type_class.matches(m_type);
    }

    //////////////////////////
    // mixin queries
    // getters will return nullptr if the mixin is not available

    mixin_index_t num_mixins() const noexcept;

    bool has(const mixin_info& info) const noexcept;
    bool has(mixin_id id) const noexcept;
    bool has(std::string_view name) const noexcept;

    void* get(const mixin_info& info) noexcept;
    const void* get(const mixin_info& info) const noexcept;
    void* get(mixin_id id) noexcept;
    const void* get(mixin_id id) const noexcept;
    void* get(std::string_view name) noexcept;
    const void* get(std::string_view name) const noexcept;

    void* get_at(mixin_index_t index) noexcept {
        if (index < num_mixins()) return unchecked_get_at(index);
        return nullptr;
    }
    const void* get_at(mixin_index_t index) const noexcept {
        if (index < num_mixins()) return unchecked_get_at(index);
        return nullptr;
    }

    using dnmx_basic_object::unchecked_get_at;

    template <typename Mixin>
    bool has() const noexcept {
        return has(g::get_mixin_info<Mixin>());
    }
    template <typename Mixin>
    Mixin* get() noexcept {
        return static_cast<Mixin*>(get(g::get_mixin_info<Mixin>()));
    }
    template <typename Mixin>
    const Mixin* get() const noexcept {
        return static_cast<const Mixin*>(get(g::get_mixin_info<Mixin>()));
    }

    /////////////////////////////////////////////////////////////////
    // memory, and mixin management

    // destroys all mixins resets type info
    // only throw on sealed objects
    void clear();
    void reset_type() { return clear(); }

    // reset to a given type
    // will call default init of new mixins
    // won't touch common mixins
    // destroys old-only mixins
    void reset_type(const type& type);

    // check whether object is empty - has no mixins
    [[nodiscard]] bool empty() const noexcept { return !m_mixin_data; }

    // allcator associated with this object (may be nullptr)
    allocator get_allocator() const noexcept { return m_allocator; }

    // seal object to prevent mutations
    // sealed status is transfered on move but not in any other case
    void seal() noexcept { m_sealed = true; }
    [[nodiscard]] bool sealed() const noexcept { return m_sealed; }

    /////////////////////////////////////////////////////////////////
    // compare

    // returns true with self
    // if object types are different, returns false
    // if object types are the same, will compare mixins with their equals funcs
    // if a mixins lacks this func, but has a compare func, it will be used
    // otherwise returns false
    bool equals(const object& other) const noexcept;

    // returns 0 with self
    // returns 0 if objects compare equal
    // returns -1 if this objects compares "less" than other
    // returns 1 if this compares "greater" than other
    // with different types: compare types lexicographically by canonical order
    // with the same type: compare mixins using their compare funcs
    // if a mixin lacks a compare func, throws object_error
    int compare(const object& other) const;

    dnmx_const_object_handle to_c_hanlde() const noexcept { return this; }
    static const object* from_c_handle(dnmx_const_object_handle ho) noexcept { return static_cast<const object*>(ho); }
    dnmx_object_handle to_c_hanlde() noexcept { return this; }
    static object* from_c_handle(dnmx_object_handle ho) noexcept { return static_cast<object*>(ho); }
private:
    friend class object_mutation;

    // null constructor for temporary objects which are supposed to be
    // initialized shortly after this call
    // with it we don't pay the price of calling domain.get_empty_type()
    // WARNING: this is a stricly i-know-what-im-doing constructor
    // many meethods of such a object will lead to a starigh-up crash
    object(std::nullptr_t, allocator alloc) noexcept;

    allocator m_allocator;

    bool m_sealed = false;

    void* safe_get_mixin(uint32_t index) noexcept;
    const void* safe_get_mixin(uint32_t index) const noexcept;

    // destroys and deallocates m_mixin_data, but does not touch type
    // first part of assign funcs and other change-type funcs
    void clear_mixin_data() noexcept;

    // move implementation
    void usurp(object& other) noexcept;
};

}
