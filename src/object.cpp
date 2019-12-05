// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include <dynamix/object.hpp>
#include <dynamix/object_type_info.hpp>
#include <dynamix/mixin_type_info.hpp>
#include <dynamix/message.hpp>
#include <dynamix/domain.hpp>
#include <dynamix/object_type_template.hpp>
#include <dynamix/exception.hpp>
#include <dynamix/allocators.hpp>
#include "dynamix/internal/mixin_data_in_object.hpp"

#include <tuple>

namespace dynamix
{

using namespace internal;

// used by objects with no mixin data, so they would
// return nullptr on get<Mixin>() without having to
// check or crashing
static mixin_data_in_object null_mixin_data;

object::object() noexcept
    : _type_info(&object_type_info::null())
    , _mixin_data(&null_mixin_data)
{
}

object::object(object_allocator* allocator)
    : _type_info(&object_type_info::null())
    , _mixin_data(&null_mixin_data)
    , _allocator(allocator)
{
    if (_allocator)
    {
        _allocator->on_set_to_object(*this);
    }
}

object::object(const object_type_template& type, object_allocator* allocator /*= nullptr*/)
    : object(allocator)
{
    type.apply_to(*this);
}

object::~object()
{
    clear();
    if (_allocator)
    {
        _allocator->release(*this);
    }
}

object::object(object&& o) noexcept
{
    usurp(std::move(o));
}

object& object::operator=(object&& o) noexcept
{
    clear();
    usurp(std::move(o));
    return *this;
}

#if DYNAMIX_OBJECT_IMPLICIT_COPY
object::object(const object& o)
    : _type_info(&object_type_info::null())
    , _mixin_data(&null_mixin_data)
{
    copy_from(o);
}

object& object::operator=(const object& o)
{
    copy_from(o);
    return *this;
}
#endif

object object::copy() const
{
    object o;
    o.copy_from(*this);
    return o;
}


void* object::internal_get_mixin(mixin_id id)
{
    return _mixin_data[_type_info->mixin_index(id)].mixin();
}

const void* object::internal_get_mixin(mixin_id id) const
{
    return _mixin_data[_type_info->mixin_index(id)].mixin();
}

bool object::internal_has_mixin(mixin_id id) const
{
    return _type_info->has(id);
}

void object::clear() noexcept
{
    for (const mixin_type_info* mixin_info : _type_info->_compact_mixins)
    {
        delete_mixin(mixin_info->id);
    }

    if(_mixin_data != &null_mixin_data)
    {
        _type_info->dealloc_mixin_data(_mixin_data, this);
        _mixin_data = &null_mixin_data;

        DYNAMIX_ASSERT(_type_info->num_objects > 0);
        --_type_info->num_objects;
    }

    _type_info = &object_type_info::null();
}

bool object::empty() const noexcept
{
    return _type_info == &object_type_info::null();
}

void object::change_type(const object_type_info* new_type, bool manage_mixins /*= true*/)
{
    const object_type_info* old_type = _type_info;
    mixin_data_in_object* old_mixin_data = _mixin_data;
    mixin_data_in_object* new_mixin_data = new_type->alloc_mixin_data(this);

    for (const mixin_type_info* mixin_info : old_type->_compact_mixins)
    {
        mixin_id id = mixin_info->id;
        if(new_type->has(id))
        {
            new_mixin_data[new_type->mixin_index(id)] = old_mixin_data[old_type->mixin_index(id)];
        }
        else if(manage_mixins)
        {
            delete_mixin(id);
        }
    }

    if(old_mixin_data != &null_mixin_data)
    {
        old_type->dealloc_mixin_data(old_mixin_data, this);
    }

    if (old_type != &object_type_info::null())
    {
        DYNAMIX_ASSERT(old_type->num_objects > 0);
        --old_type->num_objects;
    }
    if (new_type != &object_type_info::null())
    {
        ++new_type->num_objects;
    }

    _type_info = new_type;
    _mixin_data = new_mixin_data;

    if(manage_mixins)
    {
        for (const mixin_type_info* mixin_info : new_type->_compact_mixins)
        {
            size_t index = new_type->mixin_index(mixin_info->id);
            if(!new_mixin_data[index].buffer())
            {
                make_mixin(mixin_info->id, nullptr);
            }
        }
    }

    if (!empty())
    {
        // set the appropriate default message implementation virtual mixin
        mixin_data_in_object& data = _mixin_data[object_type_info::DEFAULT_MSG_IMPL_INDEX];
        data.set_buffer(reinterpret_cast<char*>(&_default_impl_virtual_mixin_data), sizeof(object*));
        data.set_object(this);
    }
}

bool object::make_mixin(mixin_id id, const void* source)
{
    DYNAMIX_ASSERT(_type_info->has(id));
    mixin_data_in_object& data = _mixin_data[_type_info->mixin_index(id)];
    DYNAMIX_ASSERT(!data.buffer());

    auto& dom = domain::instance();

    const mixin_type_info& mixin_info = dom.mixin_info(id);

    mixin_allocator* alloc = _allocator ? _allocator : mixin_info.allocator;
    char* buffer;
    size_t mixin_offset;
    std::tie(buffer, mixin_offset) = alloc->alloc_mixin(mixin_info, this);

    DYNAMIX_ASSERT(buffer);
    DYNAMIX_ASSERT(mixin_offset >= sizeof(object*)); // we should have room for an object pointer

    data.set_buffer(buffer, mixin_offset);
    data.set_object(this);

#if DYNAMIX_ADDITIONAL_METRICS
    ++mixin_info.num_mixins;
#endif

    if (!source)
    {
        alloc->construct_mixin(mixin_info, data.mixin());
    }
    else
    {
        if (!alloc->copy_construct_mixin(mixin_info, data.mixin(), source))
        {
            // so... we now have a problem
            // some mixins are constructed and some are not, while this cannot be constructed
            // we could potentially invalidate the object, and we should, too, since regular constructors
            // may throw outside of our code
            // for now, though, use a quick and dirty fix and default construct and then return false
            // thus we will have a valid object, but the non-copyable mixins won't be copied
            // finally throw after the object construction is complete
            alloc->construct_mixin(mixin_info, data.mixin());
            return false;
        }
    }

    return true;
}

void object::delete_mixin(mixin_id id)
{
    DYNAMIX_ASSERT(_type_info->has(id));
    mixin_data_in_object& data = _mixin_data[_type_info->mixin_index(id)];

    const mixin_type_info& mixin_info = domain::instance().mixin_info(id);

    mixin_allocator* alloc = _allocator ? _allocator : mixin_info.allocator;

    alloc->destroy_mixin(mixin_info, data.mixin());

    // dealocate mixin
    alloc->dealloc_mixin(data.buffer(),
        data.mixin_offset(), mixin_info, this);

#if DYNAMIX_ADDITIONAL_METRICS
    DYNAMIX_ASSERT(mixin_info.num_mixins > 0);
    --mixin_info.num_mixins;
#endif

    data.clear();
}

bool object::implements_message(feature_id id) const
{
    return !!_type_info->_call_table[id].top_bid_message;
}

bool object::implements_message_by_mixin(feature_id id) const
{
    auto& entry = _type_info->_call_table[id];

    if (!entry.top_bid_message)
    {
        // doesn't implement it at all
        return false;
    }

    const auto& msg_data = domain::instance().message_data(id);

    return entry.top_bid_message.data != msg_data.default_impl_data;
}

size_t object::message_num_implementers(feature_id id) const
{
    auto& entry = _type_info->_call_table[id];

    if (!entry.top_bid_message)
    {
        return 0;
    }

    if (domain::instance().message_data(id).mechanism == message_t::unicast)
    {
        return 1;
    }
    else
    {
        return entry.end - entry.begin;
    }
}

void object::get_message_names(std::vector<const char*>& out_message_names) const
{
    const domain& dom = domain::instance();

    for(size_t i=0; i<DYNAMIX_MAX_MESSAGES; ++i)
    {
        if(implements_message(i))
        {
            out_message_names.push_back(dom.message_data(i).name);
        }
    }
}

void object::get_mixin_names(std::vector<const char*>& out_mixin_names) const
{
    for (const mixin_type_info* mixin_info : _type_info->_compact_mixins)
    {
        out_mixin_names.push_back(mixin_info->name);
    }
}

bool object::has(mixin_id id) const noexcept
{
    if (id >= DYNAMIX_MAX_MIXINS)
        return false;
    return internal_has_mixin(id);
}

bool object::has(const char* mixin_name) const noexcept
{
    auto id = domain::instance().get_mixin_id_by_name(mixin_name);
    return has(id);
}

void* object::get(mixin_id id) noexcept
{
    if (id >= DYNAMIX_MAX_MIXINS)
        return nullptr;
    return internal_get_mixin(id);
}

const void* object::get(mixin_id id) const noexcept
{
    if (id >= DYNAMIX_MAX_MIXINS)
        return nullptr;
    return internal_get_mixin(id);
}

void* object::get(const char* mixin_name) noexcept
{
    auto id = domain::instance().get_mixin_id_by_name(mixin_name);
    return get(id);
}

const void* object::get(const char* mixin_name) const noexcept
{
    auto id = domain::instance().get_mixin_id_by_name(mixin_name);
    return get(id);
}

void object::usurp(object&& o) noexcept
{
    if (_allocator)
    {
        _allocator->release(*this);
        _allocator = nullptr;
    }

    if (o._allocator)
    {
        _allocator = o._allocator->on_move(*this, o);
        if (_allocator)
        {
            _allocator->on_set_to_object(*this);
        }
        o._allocator = nullptr;
    }

    _type_info = o._type_info;
    _mixin_data = o._mixin_data;

    for (size_t i = object_type_info::MIXIN_INDEX_OFFSET; i < _type_info->_compact_mixins.size() + object_type_info::MIXIN_INDEX_OFFSET; ++i)
    {
        _mixin_data[i].set_object(this);
    }

    if (!empty())
    {
        mixin_data_in_object& data = _mixin_data[object_type_info::DEFAULT_MSG_IMPL_INDEX];
        data.set_buffer(reinterpret_cast<char*>(&_default_impl_virtual_mixin_data), sizeof(object*));
        data.set_object(this);
    }

    // clear other object
    o._type_info = &object_type_info::null();
    o._mixin_data = &null_mixin_data;
}

void object::copy_from(const object& o)
{
    if (&o == this)
    {
        // check for self usurp
        return;
    }

    if (empty())
    {
        if (o._allocator)
        {
            if (_allocator)
            {
                _allocator->release(*this);
            }
            _allocator = o._allocator->on_copy_construct(*this, o);
            if (_allocator)
            {
                _allocator->on_set_to_object(*this);
            }
        }
    }

    if (o.empty())
    {
        clear();
        return;
    }

    if (o._type_info == _type_info)
    {
        copy_matching_from(o);
        return;
    }

    // what follows is basically copy-pasted from change_type
    // think of a way to share the code

    const object_type_info* old_type = _type_info;
    mixin_data_in_object* old_mixin_data = _mixin_data;
    mixin_data_in_object* new_mixin_data = o._type_info->alloc_mixin_data(this);

    auto& dom = domain::instance();

    enum
    {
        none, bad_assign, bad_copy_construct
    } copy_fail = none;

    for (const mixin_type_info* mixin_info : old_type->_compact_mixins)
    {
        mixin_id id = mixin_info->id;
        if (o._type_info->has(id))
        {
            auto new_index = o._type_info->mixin_index(id);
            auto& data = new_mixin_data[new_index];
            data = old_mixin_data[old_type->mixin_index(id)];

            if (!dom.mixin_info(id).copy_assignment)
            {
                copy_fail = bad_assign;
            }
            else
            {
                dom.mixin_info(id).copy_assignment(data.mixin(), o._mixin_data[new_index].mixin());
            }
        }
        else
        {
            delete_mixin(id);
        }
    }

    if (old_mixin_data != &null_mixin_data)
    {
        old_type->dealloc_mixin_data(old_mixin_data, this);
    }

    if (old_type != &object_type_info::null())
    {
        DYNAMIX_ASSERT(old_type->num_objects > 0);
        --old_type->num_objects;
    }
    if (o._type_info != &object_type_info::null())
    {
        ++o._type_info->num_objects;
    }

    _type_info = o._type_info;
    _mixin_data = new_mixin_data;

    for (const mixin_type_info* mixin_info : _type_info->_compact_mixins)
    {
        auto id = mixin_info->id;
        size_t index = _type_info->mixin_index(id);
        if (!new_mixin_data[index].buffer())
        {
            if (!make_mixin(id, o._mixin_data[index].mixin()))
            {
                copy_fail = bad_copy_construct;
            }
        }
    }

    // set the appropriate default message implementation virtual mixin
    mixin_data_in_object& data = _mixin_data[object_type_info::DEFAULT_MSG_IMPL_INDEX];
    data.set_buffer(reinterpret_cast<char*>(&_default_impl_virtual_mixin_data), sizeof(object*));
    data.set_object(this);

    if (copy_fail)
    {
        DYNAMIX_THROW_UNLESS(copy_fail == bad_assign, bad_copy_construction);
        DYNAMIX_THROW_UNLESS(copy_fail == bad_copy_construct, bad_copy_assignment);
    }
}

void object::copy_matching_from(const object& o)
{
    auto& dom = domain::instance();

    for (const mixin_type_info* info : o._type_info->_compact_mixins)
    {
        auto id = info->id;
        if (_type_info->has(id))
        {
            DYNAMIX_THROW_UNLESS(dom.mixin_info(id).copy_assignment, bad_copy_assignment);
            dom.mixin_info(id).copy_assignment(_mixin_data[_type_info->mixin_index(id)].mixin(), o._mixin_data[o._type_info->mixin_index(id)].mixin());
        }
    }
}

bool object::copyable() const noexcept
{
    for (const mixin_type_info* info : _type_info->_compact_mixins)
    {
        if (!info->copy_constructor) return false;
        if (!info->copy_assignment) return false;
    }

    return true;
}

void object::move_matching_from(object& o)
{
    auto& dom = domain::instance();

    for (auto* info : o._type_info->_compact_mixins)
    {
        auto id = info->id;
        if (_type_info->has(id))
        {
            DYNAMIX_THROW_UNLESS(dom.mixin_info(id).move_assignment, bad_move_assignment);
            dom.mixin_info(id).move_assignment(_mixin_data[_type_info->mixin_index(id)].mixin(), o._mixin_data[o._type_info->mixin_index(id)].mixin());
        }
    }
}

#if DYNAMIX_OBJECT_REPLACE_MIXIN

std::pair<char*, size_t> object::move_mixin(mixin_id id, char* buffer, size_t mixin_offset)
{
    if (id >= DYNAMIX_MAX_MIXINS)
        return std::pair<char*, size_t>(nullptr, 0);

    auto& data = _mixin_data[_type_info->mixin_index(id)];
    if (!data.mixin())
        return std::pair<char*, size_t>(nullptr, 0);

    auto& dom = domain::instance();
    const auto& mixin_info = dom.mixin_info(id);
    DYNAMIX_THROW_UNLESS(mixin_info.move_constructor, bad_mixin_move);

    auto old_data = data;

    data.set_buffer(buffer, mixin_offset);
    data.set_object(this);

    mixin_info.move_constructor(data.mixin(), old_data.mixin());

    return std::make_pair(old_data.buffer(), old_data.mixin_offset());
}

std::pair<char*, size_t> object::hard_replace_mixin(mixin_id id, char* buffer, size_t mixin_offset) noexcept
{
    DYNAMIX_ASSERT(id < DYNAMIX_MAX_MIXINS);
    auto& data = _mixin_data[_type_info->mixin_index(id)];
    DYNAMIX_ASSERT(data.mixin());

    auto ret = std::make_pair(data.buffer(), data.mixin_offset());
    data.set_buffer(buffer, mixin_offset);

    // not needed yet. It must be the user's responsibility
    // data.set_object(this);

    return ret;
}

void object::reallocate_mixins()
{
    for (const auto mixin_info : _type_info->_compact_mixins)
    {
        mixin_id id = mixin_info->id;
        DYNAMIX_THROW_UNLESS(mixin_info->move_constructor, bad_mixin_move);

        auto& data = _mixin_data[_type_info->mixin_index(id)];
        auto old_data = data;
        DYNAMIX_ASSERT(data.buffer());

        mixin_allocator* alloc = _allocator ? _allocator : mixin_info->allocator;

        auto new_buf = alloc->alloc_mixin(*mixin_info, this);

        data.set_buffer(new_buf.first, new_buf.second);
        data.set_object(this);

        mixin_info->move_constructor(data.mixin(), old_data.mixin());

        alloc->dealloc_mixin(old_data.buffer(), old_data.mixin_offset(), *mixin_info, this);
    }
}

#endif // DYNAMIX_OBJECT_REPLACE_MIXIN

} // dynamix
