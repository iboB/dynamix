// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "v1domain.hpp"
#include <dynamix/define_mixin.hpp>

namespace dynamix::v1compat {

struct none_t {};
inline none_t none;

struct user_data {
    user_data(uintptr_t d) noexcept : data(d) {}
    uintptr_t data;
};

template <typename Mixin>
class feature_parser : public util::mixin_info_data_builder<Mixin> {
    using super = util::mixin_info_data_builder<Mixin>;
public:
    feature_parser(util::mixin_info_data& data, dnmx_sv name) noexcept
        : super(data, name)
    {}

    template <typename Feature>
    feature_parser& operator&(Feature*) {
        super::template implements<Feature>();
        return *this;
    }

    feature_parser& operator&(none_t) {
        return *this;
    }

    feature_parser& operator&(user_data d) {
        super::user_data(d.data);
        return *this;
    }
};
}

#define DYNAMIX_V1_DEFINE_MIXIN(mixin, features) \
    DYNAMIX_DEFINE_MIXIN_EX(::dynamix::v1compat::domain_tag, mixin, ::dynamix::v1compat::feature_parser) & features
