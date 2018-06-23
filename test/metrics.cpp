// DynaMix
// Copyright (c) 2013-2018 Borislav Stanimirov, Zahary Karadjov
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// https://opensource.org/licenses/MIT
//
#include <dynamix/dynamix.hpp>

#if DYNAMIX_ADDITIONAL_METRICS

#include "test_mixins.hpp"

#include "doctest/doctest.h"

TEST_SUITE("obj optional metrics");

using namespace dynamix;

TEST_CASE("metrics")
{
    object o1;

    auto& mta = _dynamix_get_mixin_type_info((a*)(nullptr));
    auto& mtb = _dynamix_get_mixin_type_info((b*)(nullptr));
    auto& mtc = _dynamix_get_mixin_type_info((c*)(nullptr));

    CHECK(mta.num_mixins == 0);
    CHECK(mtb.num_mixins == 0);
    CHECK(mtc.num_mixins == 0);

    mutate(o1)
        .add<a>()
        .add<b>();

    auto t1 = o1._type_info;

    CHECK(t1->num_objects == 1);
    CHECK(mta.num_mixins == 1);
    CHECK(mtb.num_mixins == 1);
    CHECK(mtc.num_mixins == 0);

    o1.clear();

    CHECK(t1->num_objects == 0);
    CHECK(mta.num_mixins == 0);
    CHECK(mtb.num_mixins == 0);
    CHECK(mtc.num_mixins == 0);

    object o2;

    mutate(o2)
        .add<a>()
        .add<b>();

    CHECK(t1->num_objects == 1);
    CHECK(mta.num_mixins == 1);
    CHECK(mtb.num_mixins == 1);
    CHECK(mtc.num_mixins == 0);

    mutate(o1)
        .add<a>()
        .add<b>();

    CHECK(t1->num_objects == 2);
    CHECK(mta.num_mixins == 2);
    CHECK(mtb.num_mixins == 2);
    CHECK(mtc.num_mixins == 0);

    mutate(o1)
        .remove<a>()
        .add<c>();

    auto t2 = o1._type_info;

    CHECK(t1->num_objects == 1);
    CHECK(t2->num_objects == 1);
    CHECK(mta.num_mixins == 1);
    CHECK(mtb.num_mixins == 2);
    CHECK(mtc.num_mixins == 1);

}

#else
int main()
{
    // tests not applicalble for this case
    return 0;
}
#endif
