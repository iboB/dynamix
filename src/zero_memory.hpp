// DynaMix
// Copyright (c) 2013-2019 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once
#include <cstring>

namespace dynamix
{
namespace internal
{
inline void zero_memory(void* mem, size_t size)
{
    std::memset(mem, 0, size);
}
}
}
