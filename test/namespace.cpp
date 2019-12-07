// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/core.hpp>

#include "doctest/doctest.h"

TEST_SUITE("namespace");

using namespace dynamix;

namespace mixins1
{
DYNAMIX_DECLARE_MIXIN(a);
DYNAMIX_DECLARE_MIXIN(b);
}

namespace mixins2
{
DYNAMIX_DECLARE_MIXIN(a);
DYNAMIX_DECLARE_MIXIN(b);
}

TEST_CASE("mixins")
{
    object o;
    mutate(o)
        .add<mixins1::a>()
        .add<mixins1::b>()
        .add<mixins2::a>()
        .add<mixins2::b>();

    auto n1a = o.get<mixins1::a>();
    CHECK(n1a == o.get("mixins1::a"));
    auto n2a = o.get<mixins2::a>();
    CHECK(static_cast<void*>(n1a) != static_cast<void*>(n2a));
    CHECK(n2a == o.get("mixins2::a"));
}

namespace mixins1
{
class a
{

};

class b
{

};

#if DYNAMIX_USE_TYPEID
#   define NAME(x)
#else
#   define NAME(x) mixin_name(x) &
#endif


DYNAMIX_DEFINE_MIXIN(a,
    NAME("mixins1::a")
    none);
DYNAMIX_DEFINE_MIXIN(b,
    NAME("mixins1::b")
    none);
}

namespace mixins2
{

class a
{

};

class b
{

};


DYNAMIX_DEFINE_MIXIN(a,
    NAME("mixins2::a")
    none);
DYNAMIX_DEFINE_MIXIN(b,
    NAME("mixins2::b")
    none);
}