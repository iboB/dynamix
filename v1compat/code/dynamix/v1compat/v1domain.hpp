// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#pragma once
#include "api.h"
#include <dynamix/declare_domain.hpp>

namespace dynamix::v1compat {
struct domain_tag;
DYNAMIX_DECLARE_EXPORTED_DOMAIN(DYNAMIX_V1COMPAT_API, domain_tag);
}
