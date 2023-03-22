// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "object.hpp"
#include "type.hpp"
#include "mixin_info.hpp"
#include "domain.hpp"
#include "exception.hpp"
#include "object_mutation.hpp"

namespace dynamix {

object::object(domain& dom, const allocator& alloc) noexcept
    : dnmx_basic_object({&dom.get_empty_type(), nullptr})
    , m_allocator(alloc)
{}

//object::object(const object_initializer& tpl, const allocator& alloc)
//    : object(nullptr, alloc) // delegate to a noexcept ctor so our dtor gets called
//{
//    tpl.mutate(*this);
//}

object::object(const type& type, const allocator& alloc, bool create_sealed)
    : object(nullptr, alloc) // delegate to a noexcept ctor so our dtor gets called
{
    reset_type(type);
    m_sealed = create_sealed;
}

object::object(object&& o) noexcept
    : m_allocator(o.m_allocator)
{
    usurp(o);
}

object::object(std::nullptr_t, allocator alloc) noexcept
    : dnmx_basic_object({nullptr, nullptr})
    , m_allocator(alloc)
{}

object::~object() {
    clear_mixin_data();
}

object& object::operator=(object&& o) {
    if (&o == this) return *this; // prevent self-usurp
    if (m_allocator == o.m_allocator) {
        if (m_sealed) throw mutation_error("sealed object");
        clear_mixin_data();
        usurp(o);
    }
    else {
        throw mutation_error("move operator=");
    }
    return *this;
}

const type& object::get_type() const noexcept {
    assert(m_type);
    return *type::from_c_handle(m_type);
}

object object::copy(const allocator& alloc) const {
    object ret(nullptr, alloc);

    if (empty()) {
        // quickly create an empty object
        ret.m_type = m_type;
        return ret;
    }

    auto& mtype = get_type();
    // check if all our mixin can be copy constructed
    if (!mtype.copy_constructible()) throw mutation_error("missing copy init");

    perform_object_mutation(ret, mtype, [&](const mixin_info& info, mixin_index_t index, byte_t* new_mixin) {
        auto source = m_mixin_data[index].mixin;
        auto err = info.copy_init(&info, new_mixin, source);
        if (err) throw mutation_user_error("copy_init user", err);
    });

    return ret;
}

void object::copy_from(const object& o) {
    if (o.empty()) {
        if (m_sealed) throw mutation_error("sealed object");
        clear_mixin_data();
        m_type = o.m_type;
        return;
    }

    // check copy availability
    for (const auto* info : o.get_type().mixins) {
        if (get_type().has(info->id)) {
            // matching type - must be copy-assignable
            if (!info->copy_asgn) throw mutation_error("copy_from");
        }
        else {
            // o-only type - must be copy-constructible
            if (!info->copy_init) throw mutation_error("copy_from");
        }
    }

    perform_object_mutation(*this, o.get_type(),
        [&](const mixin_info& info, mixin_index_t index, byte_t* new_mixin) {
            auto source = o.m_mixin_data[index].mixin;
            auto err = info.copy_init(&info, new_mixin, source);
            if (err) throw mutation_user_error("copy_init user", err);
        },
        [&](const mixin_info& info, mixin_index_t index, byte_t* mixin) {
            auto source = o.m_mixin_data[index].mixin;
            auto err = info.copy_asgn(&info, mixin, source);
            if (err) throw mutation_user_error("copy_asgn user", err);
        }
    );
}

void object::copy_matching_from(const object& o) {
    auto& otype = o.get_type();
    auto& mtype = get_type();

    // check copy availability
    for (const auto* info : otype.mixins) {
        if (mtype.has(info->id)) {
            // matching type - must be copy-assignable
            if (!info->copy_asgn) throw mutation_error("copy_matching_from");
        }
    }

    // no rollback is possible here
    // if a copy throws, we just end-up in a semi-copied state

    for (size_t src_index = 0; src_index < otype.mixins.size(); ++src_index) {
        auto& info = *otype.mixins[src_index];
        auto own_index = mtype.index_of(info.id);
        if (own_index == invalid_mixin_index) continue;
        auto err = info.copy_asgn(&info,
            m_mixin_data[own_index].mixin,
            o.m_mixin_data[src_index].mixin
        );
        if (err) throw mutation_user_error("copy_asgn user", err);
    }
}

void object::move_matching_from(object& o, bool fallback_to_init) {
    auto& otype = o.get_type();
    auto& mtype = get_type();

    // check move availability
    for (const auto* info : otype.mixins) {
        if (mtype.has(info->id)) {
            // matching type - must be move-assignable
            if (info->move_asgn) continue;
            // .. or if fallback is allowed move-constructible
            if (fallback_to_init && info->move_init) continue;
            throw mutation_error("move_matching_from");
        }
    }

    for (size_t src_index = 0; src_index < otype.mixins.size(); ++src_index) {
        auto& info = *otype.mixins[src_index];
        auto own_index = mtype.index_of(info.id);
        if (own_index == invalid_mixin_index) continue;
        if (auto move_asgn = info.move_asgn) {
            move_asgn(&info,
                m_mixin_data[own_index].mixin,
                o.m_mixin_data[src_index].mixin
            );
        }
        else {
            assert(fallback_to_init); // should be covered above
            // fallback: destroy own mixin and move-construct from other
            if (info.destroy) info.destroy(&info, m_mixin_data[own_index].mixin);
            assert(info.move_init); // should be covered above
            info.move_init(&info,
                m_mixin_data[own_index].mixin,
                o.m_mixin_data[src_index].mixin
            );
        }
    }
}

domain& object::get_domain() const noexcept {
    return get_type().dom;
}

mixin_index_t object::num_mixins() const noexcept {
    return get_type().num_mixins();
}

bool object::has(mixin_id id) const noexcept {
    return get_type().has(id);
}
bool object::has(std::string_view name) const noexcept {
    return get_type().has(name);
}
bool object::has(const mixin_info& info) const noexcept {
    return get_type().has(info.id);
}

void* object::safe_get_mixin(uint32_t index) noexcept {
    if (index == invalid_mixin_index) return nullptr;
    return unchecked_get_at(index);
}
const void* object::safe_get_mixin(uint32_t index) const noexcept {
    if (index == invalid_mixin_index) return nullptr;
    return unchecked_get_at(index);
}

void* object::get(mixin_id id) noexcept {
    return safe_get_mixin(get_type().index_of(id));
}
const void* object::get(mixin_id id) const noexcept {
    return safe_get_mixin(get_type().index_of(id));
}
void* object::get(std::string_view name) noexcept {
    return safe_get_mixin(get_type().index_of(name));
}
const void* object::get(std::string_view name) const noexcept {
    return safe_get_mixin(get_type().index_of(name));
}
void* object::get(const mixin_info& info) noexcept {
    return get(info.id);
}
const void* object::get(const mixin_info& info) const noexcept {
    return get(info.id);
}

void object::clear() {
    if (m_sealed) throw mutation_error("sealed object");
    clear_mixin_data();
    m_type = &get_domain().get_empty_type();
}

void object::reset_type(const type& type) {
    if (type.num_mixins() == 0) {
        if (m_sealed) throw mutation_error("sealed object");
        clear_mixin_data();
        m_type = &type;
        return;
    }
    perform_object_mutation(*this, type);
}

bool object::equals(const object& other) const noexcept {
    if (this == &other) return true;
    if (m_type != other.m_type) return false;

    for (size_t index = 0; index < num_mixins(); ++index) {
        auto& info = *get_type().mixins[index];
        auto own_mixin = m_mixin_data[index].mixin;
        auto other_mixin = other.m_mixin_data[index].mixin;
        if (info.equals) {
            if (!info.equals(&info, own_mixin, other_mixin)) return false;
        }
        else if (info.compare) {
            if (info.compare(&info, own_mixin, other_mixin)) return false;
        }
        else {
            return false;
        }
    }

    return true;
}

int object::compare(const object& other) const {
    if (this == &other) return 0;

    auto& mtype = get_type();
    if (auto cmp = mtype.compare(other.get_type())) return cmp;

    for (size_t index = 0; index < mtype.mixins.size(); ++index) {
        auto& info = *mtype.mixins[index];
        if (!info.compare) throw compare_error("compare");
        auto own_mixin = m_mixin_data[index].mixin;
        auto other_mixin = other.m_mixin_data[index].mixin;
        if (auto cmp = info.compare(&info, own_mixin, other_mixin)) return cmp;
    }

    return 0;
}

void object::clear_mixin_data() noexcept {
    if (empty()) return;

    object_mutation t(nullptr, *this);
    t.rollback();
    m_mixin_data = nullptr;
}

void object::usurp(object& other) noexcept {
    m_type = other.m_type;
    m_mixin_data = other.m_mixin_data;
    m_sealed = other.m_sealed;

    // we also need to redirect mixin datas to this
    for (size_t i = 0; i < num_mixins(); ++i) {
        m_mixin_data[i].set_object(this);
    }

    // clear other object
    other.m_type = &get_domain().get_empty_type();
    other.m_mixin_data = nullptr;
}

}
