// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "v1domain.hpp"
#include <dynamix/define_mixin.hpp>

namespace dynamix {
struct none {};
namespace impl {
template <typename Mixin>
class feature_parser : public util::mixin_info_data_builder<Mixin> {
    using super = util::mixin_info_data_builder<Mixin>;
public:
    feature_parser(util::mixin_info_data& data, dnmx_sv name) noexcept
        : util::mixin_info_data_builder(data, name)
    {}

    template <typename Feature>
    feature_parser& operator&(Feature*) {
        super::template implements<Feature>();
        return *this;
    }

    feature_parser& operator&(none) {
        return *this;
    }
};
}
}

#define DYNAMIX_V1_DEFINE_MIXIN(mixin, features) \
    DYNAMIX_DEFINE_MIXIN_EX(::dynamix::v1compat::domain_tag, mixin, ::dynamix::v1compat::feature_parser) & features
