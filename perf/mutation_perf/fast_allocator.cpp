// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "common.hpp"
#include "fast_allocator.hpp"

#include <cstdlib>

using namespace std;
using namespace dynamix;

static constexpr size_t BUF_SIZE = 10 * 1024 * 1024;

fast_allocator::fast_allocator()
    : _buffer(new char[BUF_SIZE])
    , _ptr(_buffer)
{
}

fast_allocator::~fast_allocator()
{
    assert(_living_objects == 0);
    delete[] _buffer;
}

char* fast_allocator::allocate(size_t bytes)
{
    if (_ptr + bytes > _buffer + BUF_SIZE)
        exit(1);
    ++_living_objects;
    auto ret = _ptr;
    _ptr += bytes;
    return ret;
}

std::pair<char*, size_t> fast_allocator::alloc_mixin(const mixin_type_info& info, const object*)
{
    size_t mem_size = mem_size_for_mixin(info.size, info.alignment);

    auto buffer = allocate(mem_size);
    auto offset = mixin_offset(buffer, info.alignment);
    return std::make_pair(buffer, offset);
}

void fast_allocator::dealloc_mixin(char*, size_t, const mixin_type_info&, const object*)
{
    --_living_objects;
}

char* fast_allocator::alloc_mixin_data(size_t count, const object*)
{
    return allocate(mixin_data_size * count);
}

void fast_allocator::dealloc_mixin_data(char*, size_t, const object*)
{
    --_living_objects;
}
