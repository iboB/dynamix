// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "test_data.hpp"

#include <dynamix/dbg_dmp.hpp>
#include <dynamix/object.hpp>
#include <dynamix/mutate.hpp>

#include <doctest/doctest.h>

#include <sstream>

using namespace dynamix;

TEST_SUITE_BEGIN("dynamix");

// smoke tests: dumps should work and contain the relevant names
TEST_CASE("dbg_dmp") {
    test_data t;
    domain dom("dmp");
    t.register_all_mixins(dom);
    t.create_types(dom);
    dom.register_type_class(t.serializable);

    {
        std::ostringstream out;
        util::dbg_dmp(out, dom);
        auto str = out.str();
        CHECK(str.find("dmp") != std::string::npos);
        CHECK(str.find("'movable'") != std::string::npos);
        CHECK(str.find("serializable") != std::string::npos);
    }

    {
        std::ostringstream out;
        util::dbg_dmp(out, *t.t_asim);
        CHECK(out.str().find("type: {") == 0);
    }

    {
        std::ostringstream out;
        util::dbg_dmp(out, t.serializable);
        CHECK(out.str().find("type class: 'serializable', match: ") == 0);
    }

    {
        object obj(dom);
        mutate(obj, add(*t.movable), add(*t.procedural_geometry));
        obj.seal();
        std::ostringstream out;
        util::dbg_dmp(out, obj);
        auto str = out.str();
        CHECK(str.find("object (") == 0);
        CHECK(str.find("sealed") != std::string::npos);
        CHECK(str.find("type: {'movable', 'procedural_geometry'}") != std::string::npos);
        CHECK(str.find("'procedural_geometry': ") != std::string::npos);
    }

    dom.unregister_type_class(t.serializable);
}
