// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "common.hpp"
#include "custom_alloc_mixin.hpp"

using namespace std;
using namespace dynamix;

class custom_alloc_mixin
{
    double a, b; // some random values
};

// perfect allocations style allocator
// doesn't care about deallocation
struct custom_allocator : public mixin_allocator
{
    static const size_t NUM_IN_PAGE = 100000;

    const size_t mixin_unit_size;
    vector<char*> mixin_block;
    size_t mixin_index;

    custom_allocator()
        : mixin_unit_size(
            calculate_mem_size_for_mixin(
                sizeof(custom_alloc_mixin),
                std::alignment_of<custom_alloc_mixin>::value))
    {
        new_mixin_page();
    }

    void new_mixin_page()
    {
        char* page = new char[mixin_unit_size * NUM_IN_PAGE];
        mixin_block.push_back(page);
        mixin_index = 0;
    }

    virtual void alloc_mixin(size_t mixin_class_size, size_t mixin_alignment, char*& out_buffer, size_t& out_mixin_offset) override
    {
        if(mixin_index == NUM_IN_PAGE)
        {
            new_mixin_page();
        }

        out_buffer = mixin_block.back() + mixin_index * mixin_unit_size;
        ++mixin_index;

        out_mixin_offset = calculate_mixin_offset(out_buffer, mixin_alignment);
    }

    virtual void dealloc_mixin(char* ptr) override
    {
    }
};

DYNAMIX_DEFINE_MIXIN(custom_alloc_mixin, dynamix::allocator<custom_allocator>())
