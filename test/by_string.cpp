// DynaMix
// Copyright (c) 2013-2017 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#include "doctest/doctest.h"

using namespace dynamix;

TEST_SUITE("by string");

class mixin_a {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "mixin_a"; }
#endif
};
DYNAMIX_DEFINE_MIXIN(mixin_a, none);

class other_mixin {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "other_mixin"; }
#endif
};
DYNAMIX_DEFINE_MIXIN(other_mixin, none);

class third {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "third"; }
#endif
};
DYNAMIX_DEFINE_MIXIN(third, none);

class unused {
public:
#if !DYNAMIX_USE_TYPEID
    static const char* dynamix_mixin_name() { return "unused"; }
#endif
};
DYNAMIX_DEFINE_MIXIN(unused, none);

TEST_CASE("mixin_ids")
{
    using internal::domain;
    const domain& di = domain::instance();

    mixin_id aid = di.get_mixin_id_by_name("mixin_a");
    mixin_id omid = di.get_mixin_id_by_name("other_mixin");
    mixin_id tid = di.get_mixin_id_by_name("third");
    mixin_id uid = di.get_mixin_id_by_name("unused");

    mixin_id paths[] = {aid, omid, tid, uid};
    std::sort(paths, paths + 4);
    CHECK(paths[0] == 0);
    CHECK(paths[1] == 1);
    CHECK(paths[2] == 2);
    CHECK(paths[3] == 3);

    mixin_id invalid = di.get_mixin_id_by_name("asdfasdf");
    CHECK(invalid == INVALID_MIXIN_ID);

    invalid = di.get_mixin_id_by_name("MIXIN_A");
    CHECK(invalid == INVALID_MIXIN_ID);

    object o;
    single_object_mutator mut(o);

    CHECK_NOTHROW(mut.add(aid));
    CHECK_NOTHROW(mut.add(omid));
    CHECK_NOTHROW(mut.add(tid));

#if DYNAMIX_USE_EXCEPTIONS
    CHECK_THROWS_AS(mut.add(invalid), bad_mutation);
    CHECK_THROWS_AS(mut.add(1234), bad_mutation);
#endif

    mut.apply();

    CHECK(o.has<mixin_a>());
    CHECK(o.has<other_mixin>());
    CHECK(o.has<third>());
    CHECK(!o.has<unused>());

    CHECK(o.has(aid));
    CHECK(o.has(omid));
    CHECK(o.has(tid));
    CHECK(!o.has(uid));
    CHECK(!o.has(invalid));
    CHECK(!o.has(1234));

    CHECK(o.get<mixin_a>() == o.get(aid));
    CHECK(o.get<other_mixin>() == o.get(omid));
    CHECK(!o.get(uid));
    CHECK(!o.get(invalid));
}

TEST_CASE("mixin_names")
{
    object_type_template tmpl;

    CHECK(tmpl.add("mixin_a"));
    CHECK(tmpl.add("other_mixin"));
    CHECK(tmpl.add("third"));

    CHECK(!tmpl.add("asdf"));
    CHECK(!tmpl.add("mixin_axxx"));
    CHECK(!tmpl.add("MIXIN_A"));

    tmpl.create();

    object o(tmpl);

    CHECK(o.has<mixin_a>());
    CHECK(o.has<other_mixin>());
    CHECK(o.has<third>());
    CHECK(!o.has<unused>());
    CHECK(o.has("mixin_a"));
    CHECK(o.has("other_mixin"));
    CHECK(o.has("third"));
    CHECK(!o.has("unused"));

    single_object_mutator mutator(o);

    CHECK(mutator.remove("unused"));
    CHECK(mutator.remove("other_mixin"));
    CHECK(mutator.remove("third"));

    CHECK(!mutator.remove("other_mixinxxx"));
    CHECK(!mutator.remove("OTHER_MIXIN"));

    mutator.apply();

    CHECK(o.has<mixin_a>());
    CHECK(!o.has<other_mixin>());
    CHECK(!o.has<third>());
    CHECK(!o.has<unused>());
    CHECK(o.has("mixin_a"));
    CHECK(!o.has("other_mixin"));
    CHECK(!o.has("third"));
    CHECK(!o.has("unused"));
}