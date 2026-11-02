// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
// Companion example for doc/advanced/no-macros.md
//
#include <dynamix/declare_mixin.hpp>
#include <dynamix/msg/declare_msg.hpp>

#include <dynamix/domain.hpp>
#include <dynamix/object.hpp>
#include <dynamix/mutate.hpp>
#include <dynamix/mixin_info_data.hpp>
#include <dynamix/common_mixin_info.hpp>
#include <dynamix/exception.hpp>
#include <dnmx/mixin_common_funcs.h>

#include <iostream>
#include <string>

// messages don't depend on domains, so they can be declared and defined with macros as usual
DYNAMIX_DECLARE_MSG(describe_msg, describe, std::string, (const dynamix::object&));

// [classes]
struct transform {
    float x = 0, y = 0;
    std::string describe() const { return "transform at " + std::to_string(x); }
};

struct health {
    int hp = 100;
    std::string describe() const { return "health " + std::to_string(hp); }
};
// [classes]

// [typed]
// the declaration allows the typed API: add<transform>(), get<transform>()
DYNAMIX_DECLARE_MIXIN(transform);

// mixin info data: the mixin info, along with storage for its features and other data
dynamix::util::mixin_info_data transform_data;

// this is what DYNAMIX_DEFINE_MIXIN would define
const dynamix::mixin_info& _dynamix_get_mixin_info(const transform*) {
    return transform_data.info;
}
// [typed]

int main() {
    using namespace dynamix;
    using namespace dynamix::util::builder_literals; // for 1_bid and others (DYNAMIX_DEFINE_MIXIN brings them in)

    // [domain]
    domain dom("local");
    // [domain]

    // [builder]
    // the same builder which DYNAMIX_DEFINE_MIXIN uses
    util::mixin_info_data_builder<transform>(transform_data, "transform")
        .implements<describe_msg>();
    transform_data.register_in(dom);
    // [builder]

    // [untyped]
    // a mixin without a declaration can only be used by info or by name
    util::mixin_info_data health_data;
    util::mixin_info_data_builder<health>(health_data, "health")
        .implements<describe_msg>(1_bid);
    health_data.register_in(dom);
    // [untyped]

    // [use]
    object obj(dom);
    mutate(obj, add<transform>(), add("health"));
    obj.get<transform>()->x = 3;
    std::cout << describe(obj) << '\n'; // health has the higher bid

    mutate(obj, dynamix::remove("health"));
    std::cout << describe(obj) << '\n';
    // [use]

    // [runtime]
    // a mixin defined at run time: for example by a script or by data
    // it's a buffer of 16 bytes, initialized with zeroes and copied with memcpy
    std::string runtime_name = "blob";
    common_mixin_info blob(runtime_name);
    blob.set_size_alignment(16, 8);
    blob.init = dnmx_mixin_common_init_func;
    blob.copy_init = dnmx_mixin_common_copy_func;
    blob.copy_asgn = dnmx_mixin_common_copy_func;
    blob.move_init = dnmx_mixin_common_move_func;
    blob.move_asgn = dnmx_mixin_common_move_func;
    dom.register_mixin(blob);

    mutate(obj, add(blob));
    std::cout << "has blob: " << obj.has("blob") << '\n';
    // [runtime]

    // [errors]
    // explicit registration errors can be caught
    common_mixin_info other_blob(runtime_name); // duplicate name
    try {
        dom.register_mixin(other_blob);
    }
    catch (const domain_error& e) {
        std::cout << "error: " << e.what() << '\n';
    }
    // [errors]

    // [cleanup]
    // objects must not have mixins which are being unregistered
    obj.clear();
    dom.unregister_mixin(blob);
    health_data.unregister_from(dom);
    transform_data.unregister_from(dom);
    // [cleanup]

    return 0;
}

#include <dynamix/msg/define_msg.hpp>
#include <dynamix/msg/func_traits.hpp>

DYNAMIX_MAKE_FUNC_TRAITS(describe);
DYNAMIX_DEFINE_MSG(describe_msg, unicast, describe, std::string, (const dynamix::object&));
