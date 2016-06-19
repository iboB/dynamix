// DynaMix
// Copyright (c) 2013-2016 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#pragma once

#include <iostream>
#include <chrono>

#if defined(_MSC_VER) || defined(__MINGW32__)
struct high_res_clock
{
    typedef long long rep;
    typedef std::nano period;
    typedef std::chrono::duration<rep, period> duration;
    typedef std::chrono::time_point<high_res_clock> time_point;
    static const bool is_steady = true;

    static time_point now();
};
#else
typedef std::chrono::high_resolution_clock high_res_clock;
#endif

class timer
{
public:
    void start(const char* name)
    {
        _name = name;
        _enterTime = high_res_clock::now();
    }

    void avg(size_t num_tests)
    {
        auto duration = high_res_clock::now() - _enterTime;
        auto time = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

        time /= num_tests;
        std::cout << _name << " avg: " << time << " ns" << std::endl;
    }

private:
    high_res_clock::time_point _enterTime;
    const char* _name;
};