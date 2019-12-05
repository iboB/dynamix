// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include "../config.hpp"
#include "assert.hpp"

#include <cstddef>

namespace dynamix
{
class object;

namespace internal
{
// represents the mixin data in an object
class mixin_data_in_object
{
public:
    void set_buffer(char* buffer, size_t mixin_offset)
    {
        I_DYNAMIX_ASSERT(buffer);
        I_DYNAMIX_ASSERT(mixin_offset >= sizeof(object*));
        _buffer = buffer;
        _mixin = buffer + mixin_offset;
    }

    void set_object(object* o)
    {
        I_DYNAMIX_ASSERT(o);
        I_DYNAMIX_ASSERT(_buffer);
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

    size_t mixin_offset() const
    {
        return _mixin - _buffer;
    }
private:
    char* _buffer = nullptr;
    char* _mixin = nullptr;
};

}
}
