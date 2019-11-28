// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

class fast_allocator : public dynamix::object_allocator
{
public:
    fast_allocator();
    ~fast_allocator();

    virtual std::pair<char*, size_t> alloc_mixin(const dynamix::mixin_type_info& info, const dynamix::object* obj) override;
    virtual void dealloc_mixin(char* ptr, size_t mixin_offset, const dynamix::mixin_type_info& info, const dynamix::object* obj) override;
    virtual char* alloc_mixin_data(size_t count, const dynamix::object* obj) override;
    virtual void dealloc_mixin_data(char* ptr, size_t count, const dynamix::object* obj) override;

private:
    char* allocate(size_t bytes);
    char* const _buffer;
    char* _ptr;
    int _living_objects = 0;
};