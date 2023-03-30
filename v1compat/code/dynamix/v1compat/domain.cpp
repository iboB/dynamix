// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "domain.hpp"
#include <dynamix/domain_settings_builder.hpp>
#include <dynamix/define_domain.hpp>
#include <dynamix/globals.hpp>

namespace dynamix::v1compat {
DYNAMIX_DEFINE_DOMAIN(domain_tag, "v1compat", domain_settings_builder().canonicalize_types());
dynamix::domain& domain::instance() noexcept {
    return g::get_domain<domain_tag>();
}
}
