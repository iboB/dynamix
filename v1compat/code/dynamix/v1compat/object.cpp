// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "object.hpp"
#include "v1domain.hpp"
#include <dynamix/globals.hpp>
#include <dynamix/type.hpp>

namespace dynamix::v1compat {
object::object() noexcept
    : dynamix::object(dynamix::g::get_domain<domain_tag>())
{}

bool object::implements(const feature_info& f) const noexcept
{
    return get_type().implements(f);
}
}
