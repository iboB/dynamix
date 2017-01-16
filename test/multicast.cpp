// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest/doctest.h"

TEST_SUITE("multicast");

using namespace dynamix;
using namespace std;

DYNAMIX_DECLARE_MIXIN(a);
DYNAMIX_DECLARE_MIXIN(b);
DYNAMIX_DECLARE_MIXIN(c);
DYNAMIX_DECLARE_MIXIN(d);

DYNAMIX_MULTICAST_MESSAGE_1(void, trace, std::ostream&, out);
DYNAMIX_MULTICAST_MESSAGE_1(void, priority_trace, std::ostream&, out);

TEST_CASE("different_priority")
{
    object o;

    mutate(o)
        .add<a>()
        .add<b>()
        .add<c>()
        .add<d>();

    CHECK(o.implements(priority_trace_msg));
    CHECK(o.num_implementers(priority_trace_msg) == 4);

    ostringstream sout;
    priority_trace(o, sout);
    CHECK(sout.str() == "210-1");
}

TEST_CASE("same_priority")
{
    object o;

    mutate(o)
        .add<a>()
        .add<b>()
        .add<c>()
        .add<d>();

    CHECK(o.implements(trace_msg));
    CHECK(o.num_implementers(trace_msg) == 4);

    ostringstream sout;
    trace(o, sout);
    CHECK(sout.str() == "abcd");
}

class a
{
public:
    void trace(std::ostream& out)
    {
        out << "a";
    }

    void priority_trace(std::ostream& out)
    {
        out << "-1";
    }
};

class b
{
public:
    void trace(std::ostream& out)
    {
        out << "b";
    }

    void priority_trace(std::ostream& out)
    {
        out << "2";
    }
};

class c
{
public:
    void trace(std::ostream& out)
    {
        out << "c";
    }

    void priority_trace(std::ostream& out)
    {
        out << "1";
    }
};

class d
{
public:
    void trace(std::ostream& out)
    {
        out << "d";
    }

    void priority_trace(std::ostream& out)
    {
        out << "0";
    }
};

// this order should be important if the messages aren't sorted by mixin name
DYNAMIX_DEFINE_MIXIN(b, trace_msg & priority(2, priority_trace_msg));
DYNAMIX_DEFINE_MIXIN(a, trace_msg & priority(-1, priority_trace_msg));
DYNAMIX_DEFINE_MIXIN(c, trace_msg & priority(1, priority_trace_msg));
DYNAMIX_DEFINE_MIXIN(d, trace_msg & priority_trace_msg);

DYNAMIX_DEFINE_MESSAGE(trace);
DYNAMIX_DEFINE_MESSAGE(priority_trace);
