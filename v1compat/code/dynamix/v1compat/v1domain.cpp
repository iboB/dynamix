// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "v1domain.hpp"
#include <dynamix/domain_settings_builder.hpp>
#include <dynamix/define_domain.hpp>

namespace dynamix::v1compat {
DYNAMIX_DEFINE_DOMAIN(domain_tag, "v1compat", domain_settings_builder().canonicalize_types());
}
