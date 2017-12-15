// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "internal.hpp"
#include <dynamix/mixin_type_info.hpp>
#include <dynamix/object_type_info.hpp>
#include <dynamix/domain.hpp>
#include <dynamix/allocators.hpp>
#include <dynamix/exception.hpp>
#include <dynamix/object.hpp>

namespace dynamix
{
namespace internal
{

object_type_info::object_type_info()
{
    zero_memory(_mixin_indices, sizeof(_mixin_indices));
    zero_memory(_call_table, sizeof(_call_table));

    // set the end of the array to point to the virtual default impl mixin
    // since every default impl message will have this as an id,
    // it will make the correct cross index when the message is called
    _mixin_indices[DYNAMIX_MAX_MIXINS] = DEFAULT_MSG_IMPL_INDEX;
}

object_type_info::~object_type_info()
{
}

static const object_type_info null_type_info;

const object_type_info& object_type_info::null()
{
    return null_type_info;
};

mixin_data_in_object* object_type_info::alloc_mixin_data(const object* obj) const
{
    const size_t num_to_allocate = _compact_mixins.size() + MIXIN_INDEX_OFFSET;

    domain_allocator* alloc = obj->_allocator ? obj->_allocator : domain::instance().allocator();
    char* memory = alloc->alloc_mixin_data(num_to_allocate, obj);
    mixin_data_in_object* ret = new (memory) mixin_data_in_object[num_to_allocate];

    return ret;
}

void object_type_info::dealloc_mixin_data(mixin_data_in_object* data, const object* obj) const
{
    const size_t num_mixins = _compact_mixins.size() + MIXIN_INDEX_OFFSET;
    for (size_t i = MIXIN_INDEX_OFFSET; i < num_mixins; ++i)
    {
        data[i].~mixin_data_in_object();
    }

    domain_allocator* alloc = obj->_allocator ? obj->_allocator : domain::instance().allocator();
    alloc->dealloc_mixin_data(reinterpret_cast<char*>(data), num_mixins, obj);
}

void object_type_info::fill_call_table()
{
    // first pass
    // find top bid messages and prepare to calculate message buffer length length

    intptr_t message_data_buffer_size = 0;

    // in this pass we make use of the fact that _call_table begin and end start as nullptr
    // for a new type so will will use them as counters

    for (const mixin_type_info* info : _compact_mixins)
    {
        for (const message_for_mixin& msg : info->message_infos)
        {
            call_table_entry& table_entry = _call_table[msg.message->id];

            if (msg.message->mechanism == message_t::unicast)
            {
                if (!table_entry.top_bid_message)
                {
                    // new message
                    table_entry.top_bid_message = &msg;
                }
                else if (table_entry.top_bid_message->priority < msg.priority)
                {
                    // we found bigger priority
                    // make it looks like a new message
                    table_entry.top_bid_message = &msg;

                    // also remove the top-priority size we've accumulated
                    message_data_buffer_size -= reinterpret_cast<intptr_t>(table_entry.begin) / sizeof(void*);
                    table_entry.begin = nullptr;
                }
                else if (table_entry.top_bid_message->priority == msg.priority)
                {
                    if (!table_entry.begin)
                    {
                        // same-priority message
                        // we will need a buffer for those if they're top-priority
                        // add one to the buffer for the first one too
                        ++message_data_buffer_size;

                        // hacky usage of end to count top bidders
                        // we need this so we can update message_data_buffer_size if we encounter
                        // a message with a higher priority
                        ++table_entry.begin;
                    }

                    ++message_data_buffer_size;
                    ++table_entry.begin;

                    // we have multiple bidders for the same priority
                    if (table_entry.top_bid_message->bid < msg.bid)
                    {
                        table_entry.top_bid_message = &msg;
                    }
                }
            }
            if(msg.message->mechanism == message_t::multicast)
            {
                if (!table_entry.begin)
                {
                    // for each new mulicast message add one more element to the buffer
                    // it will be filled with nullptr so that we know when to stop when
                    // searching the individual message buffer for DYNAMIX_CALL_NEXT_BIDDER
                    ++message_data_buffer_size;

                    // also set top bid message just so we mark it as implemented
                    // it won't actually be used for multicasts
                    table_entry.top_bid_message = &msg;
                }

                // again we use begin to set the size of the buffer this particular message needs
                ++message_data_buffer_size;
                ++table_entry.begin;
            }
        }
    }

    const domain& dom = domain::instance();

    _message_data_buffer.reset(new pc_message_for_mixin[message_data_buffer_size]);
    auto message_data_buffer_ptr = _message_data_buffer.get();

    // second pass
    // update begin and end pointers of _call_table and add message datas to buffer
    for (const mixin_type_info* info : _compact_mixins)
    {
        for (const message_for_mixin& msg : info->message_infos)
        {
            call_table_entry& table_entry = _call_table[msg.message->id];

            if(table_entry.begin)
            {
                if (!table_entry.end)
                {
                    // begin is not null and end is null, so this message has not been updated
                    // but needs to be

                    auto begin = message_data_buffer_ptr;
                    message_data_buffer_ptr += reinterpret_cast<intptr_t>(table_entry.begin) / sizeof(void*);

                    if (msg.message->mechanism == message_t::multicast)
                    {
                        // for multicasts also add one for the nullptr terminators
                        ++message_data_buffer_ptr;
                    }

                    DYNAMIX_ASSERT(message_data_buffer_ptr - _message_data_buffer.get() <= message_data_buffer_size);
                    table_entry.begin = begin;
                    table_entry.end = begin;
                }

                if (msg.message->mechanism == message_t::multicast || table_entry.top_bid_message->priority == msg.priority)
                {
                    // add all messages for multicasts
                    // add same-bid messages for unicasts
                    *table_entry.end++ = &msg;
                }
            }
        }
    }

    // third pass through all messages of the domain
    // if we implement it AND it has a buffer, sort it
    for(size_t i=0; i<dom._num_registered_messages; ++i)
    {
        if (!dom._messages[i])
        {
            // no such message
            continue;
        }

        call_table_entry& table_entry = _call_table[i];

        if (!table_entry.begin)
        {
            // either we don't implement this message
            // or it's a unicast with the same bid
            // we don't care about those
            continue;
        }

        // sort by bid
        std::sort(table_entry.begin, table_entry.end, [](const message_for_mixin* a, const message_for_mixin* b) -> bool
        {
            // descending
            return b->bid < a->bid;
        });

        if (dom._messages[i]->mechanism == message_t::multicast)
        {
            // for multicasts we have extra work to do
            // we need to sort messages with the same bid by priority

            // we will also redirect table_entry.end to point to the first set
            // of messages (the top bidders) thus using it for multicast calls
            // and gen_num_bidders
            // we will set the actual end of the buffer to point to nullptr
            // so we know when to stop when searching through it for DYNAMIX_CALL_NEXT_BIDDER
            const message_for_mixin** first_end = nullptr;

            auto begin = table_entry.begin;
            for (auto ptr = table_entry.begin; ptr < table_entry.end; ++ptr)
            {
                auto next = ptr + 1;

                if (next == table_entry.end || (*next)->bid != (*ptr)->bid)
                {
                    // bid change
                    // sort by priority
                    std::sort(begin, next, [](const message_for_mixin* a, const message_for_mixin* b) -> bool
                    {
                        if (b->priority == a->priority)
                        {
                            const domain& dom = domain::instance();

                            // on the same priority sort by name of mixin
                            // this will guarantee that different compilations of the same mixins sets
                            // will always have the same order of multicast execution
                            const char* name_a = dom.mixin_info(a->_mixin_id).name;
                            const char* name_b = dom.mixin_info(b->_mixin_id).name;

                            return strcmp(name_a, name_b) < 0;
                        }
                        else
                        {
                            return b->priority < a->priority;
                        }
                    });

                    begin = next;

                    if (!first_end)
                    {
                        first_end = next;
                    }
                }
            }

            *table_entry.end = nullptr; // set nullptr at the end of the buffer
            table_entry.end = first_end;
        }
    }

    // pass 1.5
    // check for unicast clashes
    // no messages with the same bid at the top-priority may exist
    for (size_t i = 0; i < dom._num_registered_messages; ++i)
    {
        call_table_entry& table_entry = _call_table[i];

        if (!table_entry.begin)
        {
            // we don't implement this message
            // or a single-bid unicast
            continue;
        }

        const message_t* msg_data = dom._messages[i];

        if (!msg_data)
        {
            // no such message
            continue;
        }

        if (msg_data->mechanism != message_t::unicast)
        {
            // not a unicast
            continue;
        }

        // use this opportunity to assert that the buffer is at least two elements big
        DYNAMIX_ASSERT(table_entry.end - table_entry.begin > 1);

        for (auto ptr = table_entry.begin; ptr != table_entry.end - 1; ++ptr)
        {
            DYNAMIX_THROW_UNLESS(
                (*ptr)->bid > (*(ptr+1))->bid,
                unicast_clash
            );
        }
    }

    // final pass through all messages
    // if we don't implement a message and it has a default implementation, set it
    for (size_t i = 0; i<dom._num_registered_messages; ++i)
    {
        call_table_entry& table_entry = _call_table[i];

        if (table_entry.top_bid_message)
        {
            // we already implement this message
            continue;
        }

        const message_t* msg_data = dom._messages[i];

        if (!msg_data)
        {
            // no such message
            continue;
        }

        if (!msg_data->default_impl_data)
        {
            // message doesn't have a default implementation
            continue;
        }

        table_entry.top_bid_message = msg_data->default_impl_data;

        if (msg_data->mechanism == message_t::multicast)
        {
            // for multicasts reuse the top-bid message to hold the pointer
            // to the default implemetation
            table_entry.begin = &table_entry.top_bid_message;
            table_entry.end = table_entry.begin + 1;
        }
    }
}

} // namespace internal
} // namespace dynamix
