// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include <dynamix/domain.hpp>
#include <dynamix/domain_settings_builder.hpp>
#include <dynamix/type.hpp>

#include <doctest/doctest.h>

using namespace dynamix;

TEST_SUITE_BEGIN("dynamix");

TEST_CASE("empty") {
    // should be safe
    domain d;
    const auto mzero = mixin_id{0};
    const auto fzero = feature_id{0};

    CHECK(d.name().empty());
    CHECK_FALSE(d.settings().canonicalize_types);
    CHECK_FALSE(d.settings().allow_duplicate_feature_names);
    CHECK_FALSE(d.settings().allow_duplicate_mixin_names);
    CHECK(d.user_data == 0);
    CHECK_FALSE(d.context);

    CHECK_FALSE(d.get_feature_info(fzero));
    CHECK_FALSE(d.get_mixin_info(mzero));

    CHECK(d.num_types() == 0);
    CHECK(d.num_type_queries() == 0);

    d.garbage_collect_types(); // should be safe

    auto& t = d.get_empty_type();
    CHECK(t.compare(t) == 0);
    CHECK(&d.get_type(itlib::span<const mixin_info*>{}) == &t);

    CHECK(&t.dom == &d);

    CHECK(t.num_mixins() == 0);
    CHECK(t.ftable_length == 0);
    CHECK(t.mixins.empty());
    CHECK(t.object_buffer_size == 0);
    CHECK(t.object_buffer_alignment == 0);
    CHECK(t.mixin_offsets.empty());
    CHECK(t.sparse_mixin_indices.empty());

    CHECK(t.default_constructible());
    CHECK(t.copy_assignable());
    CHECK(t.copy_constructible());
    CHECK(t.copyable());
    CHECK(t.equality_comparable());
    CHECK(t.comparable());

    CHECK(t.index_of(mzero) == invalid_mixin_index);
    CHECK(t.index_of("aaa") == invalid_mixin_index);
    CHECK_FALSE(t.has(mzero));
    CHECK_FALSE(t.has("bbb"));

    CHECK_FALSE(t.implements_strong(fzero));
    CHECK_FALSE(t.implements_strong("xx"));
}

TEST_CASE("empty 2") {
    domain_settings s = {};
    s.canonicalize_types = true;
    s.allow_duplicate_feature_names = true;
    s.allow_duplicate_mixin_names = true;
    int ctx;
    domain d("dom", s, 1234, &ctx);
    CHECK(d.name() == "dom");
    CHECK(d.settings().canonicalize_types);
    CHECK(d.settings().allow_duplicate_feature_names);
    CHECK(d.settings().allow_duplicate_mixin_names);
    CHECK(d.user_data == 1234);
    CHECK(d.context == &ctx);
}

TEST_CASE("domain setgings builder") {
    {
        domain d("t", domain_settings_builder().canonicalize_types());
        CHECK(d.settings().canonicalize_types);
        CHECK_FALSE(d.settings().allow_duplicate_feature_names);
        CHECK_FALSE(d.settings().allow_duplicate_mixin_names);
    }
    {
        domain d("t", domain_settings_builder().allow_duplicate_feature_names());
        CHECK_FALSE(d.settings().canonicalize_types);
        CHECK(d.settings().allow_duplicate_feature_names);
        CHECK_FALSE(d.settings().allow_duplicate_mixin_names);
    }
    {
        domain d("t", domain_settings_builder().allow_duplicate_mixin_names());
        CHECK_FALSE(d.settings().canonicalize_types);
        CHECK_FALSE(d.settings().allow_duplicate_feature_names);
        CHECK(d.settings().allow_duplicate_mixin_names);
    }
    {
        domain d("t", domain_settings_builder().allow_duplicate_mixin_names().canonicalize_types());
        CHECK(d.settings().canonicalize_types);
        CHECK_FALSE(d.settings().allow_duplicate_feature_names);
        CHECK(d.settings().allow_duplicate_mixin_names);
    }
}
