// DynaMix
// Copyright (c) 2013-2020 Borislav Stanimirov, Zahary Karadjov
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

#include "config.hpp"
#include "mixin_collection.hpp"
#include "message.hpp"
#include "internal/assert.hpp"

#include <memory>
#include <cstdint>

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

    uint32_t mixin_index(mixin_id id) const { return _mixin_indices[id]; }

    static const object_type_info& null();

    mixin_data_in_object* alloc_mixin_data(const object* obj) const;
    void dealloc_mixin_data(mixin_data_in_object* data, const object* obj) const;

    bool implements_message(feature_id id) const { return !!_call_table[id].top_bid_message; }

    // the following need to be public in order for the message macros to work
_dynamix_internal:
    using mixin_collection::_mixins;
    using mixin_collection::_compact_mixins;

    // indices in the object::_mixin_data
    uint32_t _mixin_indices[DYNAMIX_MAX_MIXINS];

    // special indices in an object's _mixin_data member
    enum reserved_mixin_indices : uint32_t
    {
        // index 0 is reserved for a null mixin data. It's used to return nullptr on queries for non member mixins
        //         (without having to check with an if or worse yet - a loop)
        NULL_MIXIN_DATA_INDEX,

        // index 1 is reserved for a virtual mixin. It's used to be cast to the default message implementators
        DEFAULT_MSG_IMPL_INDEX,

        // offset of the mixin indices in the object's _mixin_data member
        MIXIN_INDEX_OFFSET
    };

    // message data for the call table which consists of tighly packed elements
    // for faster acciess
    struct call_table_message
    {
        uint32_t mixin_index; // index of mixin within the _compact_mixins vector
        func_ptr caller;
        const message_for_mixin* data;

        explicit operator bool() const { return !!caller; }
        void reset()
        {
            mixin_index = ~0u;
            caller = nullptr;
            data = nullptr;
        }
    };

    call_table_message make_call_table_message(mixin_id id, const message_for_mixin& data) const;

    struct call_table_entry
    {
        // used when building the buffer to hold the top-bid message for the top priority
        // also used in the unicast message macros for optimization - to call the top-bid
        // message without the indrection from dereferencing begin
        // also for multicasts which fall back to a default msg implementation this is used to
        // hold the pointer to the default implementation
        call_table_message top_bid_message;

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
        call_table_message* begin;
        call_table_message* end;
    };

    // a single buffer for all dynamically allocated message pointers to minimize allocations
    std::unique_ptr<call_table_message[]> _message_data_buffer;
    call_table_entry _call_table[DYNAMIX_MAX_MESSAGES];

    // number of living objects with this type info
    mutable metric num_objects = {size_t(0)};

    // this should be called after the mixins have been initialized
    void fill_call_table();
};

} // namespace internal
} // namespace dynamix
