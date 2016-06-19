// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include "timer.hpp"

#if defined(_MSC_VER) || defined(__MINGW32__)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

const long long FREQ = []() -> long long
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    return frequency.QuadPart;
}();

high_res_clock::time_point high_res_clock::now()
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return time_point(duration((t.QuadPart * rep(period::den)) / FREQ));
}

#endif