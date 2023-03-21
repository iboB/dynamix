// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "bits/make_nullptr.hpp"
#include "mixin_info_fwd.hpp"
#include "feature_info_fwd.hpp"

namespace dynamix {
class domain;
class common_feature_info;

namespace g {
template <typename DomainTag>
domain& get_domain() noexcept { return _dynamix_get_domain(impl::make_nullptr<DomainTag>()); }

template <typename Mixin>
const mixin_info& get_mixin_info() noexcept { return _dynamix_get_mixin_info(impl::make_nullptr<Mixin>()); }

template <typename Feature>
const common_feature_info& get_feature_info_safe() noexcept { return Feature::get_info_safe(); }

template <typename Feature>
const feature_info& get_feature_info_fast() noexcept { return Feature::info; }
}
}
