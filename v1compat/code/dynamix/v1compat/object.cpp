// Copyright (c) Borislav Stanimirov
// SPDX-License-Identifier: MIT
//
#include "object.hpp"
#include "domain.hpp"
#include <dynamix/type.hpp>

namespace dynamix::v1compat {
object::object() noexcept
    : dynamix::object(domain::instance())
{}

bool object::implements(const feature_info& f) const noexcept
{
    return get_type().implements(f);
}
}
