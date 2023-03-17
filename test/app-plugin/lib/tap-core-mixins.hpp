// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "tap-api.h"
#include <dynamix/declare_mixin.hpp>
#include <dynamix/common_mixin_init.hpp>

#include <string_view>

namespace tap {
DYNAMIX_DECLARE_EXPORTED_MIXIN(TAP_API, struct person);

struct TAP_API mixin_person : public dynamix::common_mixin_init<person> {
    std::string_view name;
    int age = 0;
    mixin_person(std::string_view name) : name(name) {}
    mixin_person(std::string_view name, int age) : name(name), age(age) {}
    virtual void do_init(const dynamix::mixin_info&, dynamix::mixin_index_t, dynamix::byte_t* new_mixin) final override;
};

DYNAMIX_DECLARE_EXPORTED_MIXIN(TAP_API, struct teacher);
}
