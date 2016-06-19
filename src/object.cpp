// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
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

namespace dynamix
{

using namespace internal;

// used by objects with no mixin data, so they would
// return nullptr on get<Mixin>() without having to
// check or crashing
static mixin_data_in_object null_mixin_data;

object::object()
    : _type_info(&object_type_info::null())
    , _mixin_data(&null_mixin_data)
{
}

object::object(const object_type_template& type)
    : _type_info(&object_type_info::null())
    , _mixin_data(&null_mixin_data)
{
    type.apply_to(*this);
}

object::object(object&& o)
    : _type_info(o._type_info)
    , _mixin_data(o._mixin_data)
{
    for (size_t i = object_type_info::MIXIN_INDEX_OFFSET; i < _type_info->_compact_mixins.size() + object_type_info::MIXIN_INDEX_OFFSET; ++i)
    {
        _mixin_data[i].set_object(this);
    }

    mixin_data_in_object& data = _mixin_data[object_type_info::DEFAULT_MSG_IMPL_INDEX];
    data.set_buffer(reinterpret_cast<char*>(&_default_impl_virtual_mixin_data), sizeof(object*));
    data.set_object(this);

    // clear other object
    o._type_info = &object_type_info::null();
    o._mixin_data = &null_mixin_data;
}

object::~object()
{
    clear();
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

void object::clear()
{
    for (const mixin_type_info* mixin_info : _type_info->_compact_mixins)
    {
        destroy_mixin(mixin_info->id);
    }

    if(_mixin_data != &null_mixin_data)
    {
        _type_info->dealloc_mixin_data(_mixin_data);
        _mixin_data = &null_mixin_data;
    }

    _type_info = &object_type_info::null();
}

bool object::empty() const
{
    return _type_info == &object_type_info::null();
}

void object::change_type(const object_type_info* new_type, bool manage_mixins /*= true*/)
{
    const object_type_info* old_type = _type_info;
    mixin_data_in_object* old_mixin_data = _mixin_data;
    mixin_data_in_object* new_mixin_data = new_type->alloc_mixin_data();

    for (const mixin_type_info* mixin_info : old_type->_compact_mixins)
    {
        mixin_id id = mixin_info->id;
        if(new_type->has(id))
        {
            new_mixin_data[new_type->mixin_index(id)] = old_mixin_data[old_type->mixin_index(id)];
        }
        else if(manage_mixins)
        {
            destroy_mixin(id);
        }
    }

    if(old_mixin_data != &null_mixin_data)
    {
        old_type->dealloc_mixin_data(old_mixin_data);
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
                construct_mixin(mixin_info->id);
            }
        }
    }

    // set the appropriate default message implementation virtual mixin
    mixin_data_in_object& data = _mixin_data[object_type_info::DEFAULT_MSG_IMPL_INDEX];
    data.set_buffer(reinterpret_cast<char*>(&_default_impl_virtual_mixin_data), sizeof(object*));
    data.set_object(this);
}

void object::construct_mixin(mixin_id id)
{
    DYNAMIX_ASSERT(_type_info->has(id));
    mixin_data_in_object& data = _mixin_data[_type_info->mixin_index(id)];
    DYNAMIX_ASSERT(!data.buffer());

    domain& dom = domain::instance();

    const mixin_type_info& mixin_info = dom.mixin_info(id);

    char* buffer;
    size_t mixin_offset;
#if defined(DYNAMIX_DEBUG)
    buffer = nullptr;
    mixin_offset = 0;
#endif
    mixin_info.allocator->alloc_mixin(mixin_info.size, mixin_info.alignment, buffer, mixin_offset);

    DYNAMIX_ASSERT(buffer);
    DYNAMIX_ASSERT(mixin_offset >= sizeof(object*)); // we should have room for an object pointer

    data.set_buffer(buffer, mixin_offset);
    data.set_object(this);

    dom.mixin_info(id).constructor(data.mixin());
}

void object::destroy_mixin(mixin_id id)
{
    DYNAMIX_ASSERT(_type_info->has(id));
    mixin_data_in_object& data = _mixin_data[_type_info->mixin_index(id)];

    const mixin_type_info& mixin_info = domain::instance().mixin_info(id);

    mixin_info.destructor(data.mixin());

    // dealocate mixin
    mixin_info.allocator->dealloc_mixin(data.buffer());

    data.clear();
}

bool object::implements_message(feature_id id) const
{
    return !!_type_info->_call_table[id].message_data;
}

void object::get_message_names(std::vector<const char*>& out_message_names) const
{
    const domain& dom = domain::instance();

    for(size_t i=0; i<DYNAMIX_MAX_MESSAGES; ++i)
    {
        if(implements_message(i))
        {
            out_message_names.push_back(dom._messages[i]->name);
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

bool object::has(mixin_id id) const
{
    if (id >= DYNAMIX_MAX_MIXINS)
        return false;
    return internal_has_mixin(id);
}

void* object::get(mixin_id id)
{
    if (id >= DYNAMIX_MAX_MIXINS)
        return nullptr;
    return internal_get_mixin(id);
}

const void* object::get(mixin_id id) const
{
    if (id >= DYNAMIX_MAX_MIXINS)
        return nullptr;
    return internal_get_mixin(id);
}

} // dynamix
