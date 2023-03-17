// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "tap-core-mixins.hpp"
#include "tap-messages.hpp"
#include "tap-messages-traits.hpp"
#include "tap-domain.hpp"
#include <dynamix/define_mixin.hpp>

namespace tap {

struct person {
    std::string name;
    int age;
    person(std::string_view name, int age)
        : name(name)
        , age(age)
    {}

    std::string& modify_self(int new_age) {
        age = new_age;
        return name;
    }

    void dump(std::vector<std::string>& vec) const {
        vec.push_back(name);
        vec.back() += ' ';
        vec.back() += std::to_string(age);
    }
};

void mixin_person::do_init(const dynamix::mixin_info&, dynamix::mixin_index_t, dynamix::byte_t* new_mixin) {
    new (new_mixin) person(name, age);
}

DYNAMIX_DEFINE_MIXIN(dynamix_domain, person)
    .implements_by<simple_uni>([](person* p, int na) -> std::string& { return p->modify_self(na); })
    .implements<dump_msg>()
;

struct teacher {
    std::string target;
    void set_target(std::string_view tgt) {
        target = tgt;
    }
    std::string work(char info) const {
        if (target.empty()) throw std::runtime_error("no target");
        return "teaching " + target + ' ' + info;
    }
    void dump(std::vector<std::string>& vec) const {
        vec.push_back("teacher");
    }
};

DYNAMIX_DEFINE_MIXIN(dynamix_domain, teacher)
    .implements<set_target_msg>()
    .implements<work_msg>()
    .implements<dump_msg>()
;
}
