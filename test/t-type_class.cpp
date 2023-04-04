// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "test_data.hpp"

#include <dynamix/exception.hpp>
#include <dynamix/type.hpp>
#include <dynamix/object_mixin_data.hpp>

#include <doctest/doctest.h>

using namespace dynamix;

TEST_SUITE_BEGIN("dynamix");

TEST_CASE("type class") {
    test_data t;
    domain dom;
    t.register_all_mixins(dom);
    t.create_types(dom);
    t.create_more_types(dom);
    t.create_reordered_types(dom);

    CHECK(t.t_asim->is_of(t.serializable));
    CHECK(t.t_pp->is_of(t.renderable));
    CHECK_FALSE(t.t_mov->is_of(t.serializable));
    CHECK_FALSE(t.t_acp->is_of(t.renderable));

    dom.register_type_class(t.serializable);

    CHECK(t.t_siee->is_of("serializable"));
    CHECK_FALSE(t.t_pp->is_of("serializable"));

    CHECK_THROWS_WITH_AS((void)t.t_pp->is_of("renderable"), "unknown type class", dynamix::domain_error);

    type_class tc = {};
    CHECK_THROWS_WITH_AS(dom.register_type_class(tc), "empty type_class name", dynamix::domain_error);

    tc.name = dnmx_make_sv_lit("custom");
    CHECK_THROWS_WITH_AS(dom.register_type_class(tc), "type_class missing matches func", dynamix::domain_error);

    tc.matches = [](dnmx_type_handle th) noexcept {
        auto t = type::from_c_handle(th);
        return t->has("mesh");
    };

    CHECK(t.t_impsa->is_of(tc));
    CHECK_FALSE(t.t_pp->is_of(tc));

    dom.register_type_class(tc);
    CHECK(t.t_afmi->is_of("custom"));
    CHECK_FALSE(t.t_acp->is_of("custom"));

    CHECK_THROWS_WITH_AS(dom.register_type_class(tc), "duplicate type_class name", dynamix::domain_error);
}
