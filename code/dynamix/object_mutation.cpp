// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "object_mutation.hpp"

#include "object.hpp"
#include "mixin_allocator.hpp"
#include "throw_exception.hpp"

#include <cassert>

namespace dynamix {

object_mutation::object_mutation(object& obj, const type& type)
    : object_mutation(obj, type, 0)
    // here we delegate to the basic init constructor
    // thus in the body of THIS constructor the object will count as being completely constructed
    // then if allocate_mixin_data throws an exception,
    // the destructor will be called and it will rollback successfully
{
    if (m_object.m_sealed) throw_exception::obj_mut_sealed_object(obj.get_type(), "mutate");
    allocate_mixin_data();
}

object_mutation::object_mutation(object& obj, const type& type, int) noexcept
    : m_object(obj)
    , m_old_type(type::from_c_handle(obj.m_type))
    , m_old_mixin_data(obj.m_mixin_data)
    , m_target_type(type)
    , m_target_mixin_data(nullptr)
    , m_allocated_upto(0)
    , m_updated_upto(0)
{
    // mixin data is allocated at this point by the delagator constructor
}

// "inverse" init
object_mutation::object_mutation(std::nullptr_t, object& obj) noexcept
    : m_object(obj)
    , m_old_type(nullptr)
    , m_old_mixin_data(nullptr)
    , m_target_type(obj.get_type())
    , m_target_mixin_data(obj.m_mixin_data)
    , m_allocated_upto(obj.get_type().num_mixins())
    , m_updated_upto(obj.get_type().num_mixins())
{}

object_mutation::~object_mutation() noexcept(false) {
    if (m_moved_out_from) return;

    if (std::uncaught_exceptions()) {
        rollback();
        return;
    }

    try {
        default_construct_each_new_mixin();
    }
    catch (...) {
        rollback();
        throw; // forward the exception to the outside world
    }

    assert(m_complete);
    finalize();
}

byte_t* object_mutation::allocate_external_mixin(const mixin_info& info) {
    assert(info.external());
    assert(!info.allocator);
    return static_cast<byte_t*>(m_object.m_allocator.allocate_bytes(
        info.obj_buf_size,
        info.obj_buf_alignment_and_mixin_offset
    ));
}

void object_mutation::deallocate_external_mixin(void* ptr, const mixin_info& info) noexcept {
    assert(info.external());
    assert(!info.allocator);
    m_object.m_allocator.deallocate_bytes(
        ptr,
        info.obj_buf_size,
        info.obj_buf_alignment_and_mixin_offset
    );
}

void object_mutation::allocate_mixin_data() {
    assert(m_allocated_upto == 0);

    if (m_target_type == *m_old_type) {
        // tag: same_type_shortcut
        // we've somehow ended-up mutating the object with the same type
        // in this case we know that there are no new mixins and only common ones
        // we can skip allocating new mixin data altogether
        m_target_mixin_data = m_old_mixin_data;
        return;
    }

    // * allocate using object allocator
    // * fill object pointer appropriately to newly allocated mixins
    // * attach common external mixins to mixin_data elements
    // * no mixin initialization here

    auto buf = m_target_type.allocate_object_buffer(m_object.m_allocator);
    m_target_mixin_data = reinterpret_cast<object_mixin_data*>(buf);

    for (mixin_index_t& ti = m_allocated_upto; ti < m_target_type.num_mixins(); ++ti) {
        auto offset = m_target_type.mixin_offsets[ti];
        auto& data = m_target_mixin_data[ti];
        if (offset) {
            // offset is non-zero which means an internal mixin in the unified buffer
            data.buf = nullptr;
            data.mixin = buf + offset;
            data.set_object(&m_object);
        }
        else {
            // zero offset means an external mixin
            const auto& info = *m_target_type.mixins[ti];
            if (m_old_mixin_data && m_old_type->has(info.id)) {
                // we have that external mixin, so just redirect buffer
                data = m_old_mixin_data[m_old_type->sparse_mixin_indices[info.iid()]];
            }
            else {
                if (auto m_alloc = info.allocator) {
                    // new mixin has allocator, so use it to allocate a new external buffer
                    data.buf = mixin_allocator::from_c_handle(m_alloc)->alloc_mixin_buf(info);
                }
                else {
                    // new mixin with no allocator => use our own
                    data.buf = allocate_external_mixin(info);
                }

                data.mixin = data.buf + info.obj_buf_alignment_and_mixin_offset;
                data.set_object(&m_object);
            }
        }
    }
}

void object_mutation::deallocate_mixin_data() noexcept {
    assert(m_updated_upto == 0);

    // reverse order of allocation
    for (mixin_index_t ti = m_allocated_upto; ti-- > 0; ) {
        auto& data = m_target_mixin_data[ti];
        if (!data.buf) continue; // part of the internal buffer and deallocated with it

        const auto& info = *m_target_type.mixins[ti];

        if (m_old_mixin_data && m_old_type->has(info.id)) {
            // we have that type
            // it was not allocated here
            continue;
        }

        if (auto* alloc = info.allocator) {
            // allocated by mixin allocator
            mixin_allocator::from_c_handle(alloc)->dealloc_mixin_buf(data.buf, info);
        }
        else {
            // allocated by the own allocator
            deallocate_external_mixin(data.buf, info);
        }
    }
    // finally deallocate ret
    m_target_type.deallocate_object_buffer(m_object.m_allocator, m_target_mixin_data);
    m_target_mixin_data = nullptr;
    m_allocated_upto = 0;
}

void object_mutation::destroy_new_mixins() noexcept {
    // reverse order of construction
    for (mixin_index_t ti = m_updated_upto; ti-- > 0; ) {
        const auto& info = *m_target_type.mixins[ti];
        if (m_old_mixin_data && m_old_type->has(info.id)) continue; // not new
        if (!info.destroy) continue; // no destructor
        info.destroy(&info, m_target_mixin_data[ti].mixin);
    }
    m_updated_upto = 0;
}

void object_mutation::rollback() noexcept {
    m_complete = false;
    if (m_target_type != *m_old_type) {
        // tag: same_type_shortcut
        // no new mixins, no allocated mixin data
        destroy_new_mixins();
        deallocate_mixin_data();
    }
    m_moved_out_from = true;
}

void object_mutation::finalize() noexcept {
    if (!m_complete) return;

    if (m_target_type == *m_old_type) {
        // tag: same_type_shortcut
        m_old_mixin_data = nullptr;
        m_old_type = nullptr;
        return;
    }

    // free old data if any
    if (m_old_mixin_data) {
        assert(m_old_type);
        for (mixin_index_t oi = 0; oi < m_old_type->num_mixins(); ++oi) {
            const auto& info = *m_old_type->mixins[oi];
            auto& old_data = m_old_mixin_data[oi];

            if (m_target_type.has(info.id)) {
                // common mixin
                if (old_data.buf) continue; // external: already in new buf

                // common internal mixin: move to new buf
                auto* new_mixin = m_target_mixin_data[m_target_type.sparse_mixin_indices[info.iid()]].mixin;
                info.move_init(&info, new_mixin, old_data.mixin);

                // desroy the one we moved-out from
                if (auto destroy = info.destroy) {
                    destroy(&info, old_data.mixin);
                }
            }
            else {
                // old-only mixin: destroy
                if (auto destroy = info.destroy) {
                    destroy(&info, old_data.mixin);
                }

                // ... and dealloc
                if (!old_data.buf) continue; // internal: no dealloc needed

                if (auto* alloc = info.allocator) {
                    // allocated by mixin allocator
                    mixin_allocator::from_c_handle(alloc)->dealloc_mixin_buf(old_data.buf, info);
                }
                else {
                    // allocated by the own allocator
                    deallocate_external_mixin(old_data.buf, info);
                }
            }
        }

        // deallocate old unified buffer
        m_old_type->deallocate_object_buffer(m_object.m_allocator, m_old_mixin_data);
        m_old_mixin_data = nullptr;
        m_old_type = nullptr;
    }

    m_object.m_type = &m_target_type;
    m_object.m_mixin_data = m_target_mixin_data;
}

namespace util {
void default_init_new_func(init_new_args args) {
    auto& info = args.info;
    if (!info.init) throw_exception::obj_mut_error(args.target_type, "mutate to", "missing default init", info);
    auto err = info.init(&info, args.mixin_buf);
    if (err) throw_exception::obj_mut_user_error(args.target_type, "mutate to", "deafult init", info, err);
}
}

void object_mutation::default_construct_each_new_mixin(mixin_index_t upto_index) {
    construct_each_new_mixin(util::default_init_new_func, upto_index);
}

void object_mutation::throw_bad_piecewise_mutation() {
    throw_exception::obj_error(m_target_type, "piecewise mutation out of order while mtating to");
}

}
