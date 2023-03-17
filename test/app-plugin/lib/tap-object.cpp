// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "tap-object.hpp"
#include "tap-domain.hpp"
#include <dynamix/globals.hpp>

namespace tap {
object::object() noexcept
    : dynamix::object(dynamix::g::get_domain<dynamix_domain>())
{}
}
