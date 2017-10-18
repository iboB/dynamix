// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

/**
 * \file
 * Defines internal classes that contain the type information for an object -
 * mixins, implemented features, etc.
 */

#include "global.hpp"
#include "mixin_collection.hpp"
#include "message.hpp"
#include "assert.hpp"

#include <memory>

// object type info is an immutable class that represents the type information for a
// group of objects

namespace dynamix
{

class object_mutator;
class object;

namespace internal
{

class mixin_data_in_object;

class DYNAMIX_API object_type_info : private mixin_collection
{
public:
    object_type_info();
    ~object_type_info();

    using mixin_collection::has;

    const mixin_collection* as_mixin_collection() const { return this; }

    size_t mixin_index(mixin_id id) const { return _mixin_indices[id]; }

    static const object_type_info& null();

    mixin_data_in_object* alloc_mixin_data() const;
    void dealloc_mixin_data(mixin_data_in_object* data) const;

_dynamix_internal:
    using mixin_collection::_mixins;
    using mixin_collection::_compact_mixins;

    // indices in the object::_mixin_data
    // size+1 for the id of the fake mixin used for default message implementations
    size_t _mixin_indices[DYNAMIX_MAX_MIXINS + 1];

    // special indices in an object's _mixin_data member
    enum reserved_mixin_indices
    {
        // index 0 is reserved for a null mixin data. It's used to return nullptr on queries for non member mixins
        //         (without having to check with an if or worse yet - a loop)
        NULL_MIXIN_DATA_INDEX,

        // index 1 is reserved for a virtual mixin. It's used to be cast to the default message implementators
        DEFAULT_MSG_IMPL_INDEX,

        // offset of the mixin indices in the object's _mixin_data member
        MIXIN_INDEX_OFFSET
    };

    struct call_table_entry
    {
        // used when building the buffer to hold the top-bid message for the top priority
        // also used in the unicast message macros for optimization - to call the top-bid
        // message without the indrection from dereferencing begin
        // also for multicasts which fall back to a default msg implementation this is used to
        // hold the pointer to the default implementation
        const message_for_mixin* top_bid_message;

        // a dynamically allocated array of all message datas
        // for unicasts it will hold pointers to all top-prirority messages for each bid
        // or be nullptr if there are no bids except a single one. It's used for DYNAMIX_CALL_NEXT_BIDDER
        // for multicasts it will hold groups of message datas sorted by priority sorted by bid
        // thus calling DYNAMIX_CALL_NEXT_BIDDER will result in a search in this array
        // (being progressively slower for the deph of bidders we use)
        // WARNING: for multicasts end points to the top-bid end only
        // when multiple bids are involved the buffer will continue after end until a nullptr address is pointed
        // also for multicasts it will be even slower depending on how many messages with the same bid exist
        // we pay this price to achieve the maximum performance for the straight-forward simple message call case
        const message_for_mixin** begin;
        const message_for_mixin** end;
    };

    // a single buffer for all dynamically allocated message pointers to minimize allocations
    using c_message_for_mixin = const message_for_mixin;
    using pc_message_for_mixin = c_message_for_mixin*;
    std::unique_ptr<pc_message_for_mixin[]> _message_data_buffer;
    call_table_entry _call_table[DYNAMIX_MAX_MESSAGES];

#if DYNAMIX_ADDITIONAL_METRICS
    // number of living objects with this type info
    mutable size_t num_objects = 0;
#endif

    // this should be called after the mixins have been initialized
    void fill_call_table();
};

// represents the mixin data in an object
class mixin_data_in_object
{
public:
    mixin_data_in_object()
        : _buffer(nullptr)
        , _mixin(nullptr)
    {
    }

    void set_buffer(char* buffer, size_t mixin_offset)
    {
        DYNAMIX_ASSERT(buffer);
        DYNAMIX_ASSERT(mixin_offset >= sizeof(object*));
        _buffer = buffer;
        _mixin = buffer + mixin_offset;
    }

    void set_object(object* o)
    {
        DYNAMIX_ASSERT(o);
        DYNAMIX_ASSERT(_buffer);
        object** data_as_objec_ptr = reinterpret_cast<object**>(_mixin - sizeof(object*));
        *data_as_objec_ptr = o;
    }

    void clear()
    {
        _buffer = nullptr;
        _mixin = nullptr;
    }

    char* buffer() { return _buffer; }
    void* mixin() { return _mixin; }
    const char* buffer() const { return _buffer; }
    const void* mixin() const { return _mixin; }
private:
    char* _buffer;
    char* _mixin;
};

} // namespace internal
} // namespace dynamix
